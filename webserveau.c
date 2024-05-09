#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <netinet/ip.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/epoll.h>
#include <string.h>
#include <netdb.h>

#define NBR_CLIENTS 10

typedef	struct s_client {
	int				fd;
	struct sockaddr	addr;
	socklen_t		addr_size;
	char			addr_str[16];
}				t_client;

int	error(char *str, t_client* clients, int passive_socket)
{
	printf("EXIT\n");
	perror(str);
	if (passive_socket)
		close(passive_socket);
	if (clients == NULL)
		exit(1);
	for (int i = 0; i < NBR_CLIENTS; i++)
		if (clients[i].fd >= 0)
			close(clients[i].fd);
	exit(1);
}

char *get_ip_str(const struct sockaddr *sa, char *s, size_t maxlen)
{
	switch(sa->sa_family) {
		case AF_INET:
			inet_ntop(AF_INET, &(((struct sockaddr_in *)sa)->sin_addr),
					s, maxlen);
			break;

		case AF_INET6:
			inet_ntop(AF_INET6, &(((struct sockaddr_in6 *)sa)->sin6_addr),
					s, maxlen);
			break;

		default:
			strncpy(s, "Unknown AF", maxlen);
			return NULL;
	}
	return s;
}

int	create_listen_socket(void)
{
	int	passive_sock = 0, val = 1;
	struct	addrinfo	*addr, hints;
	memset(&hints, 0, sizeof(hints));
	hints.ai_flags = AI_PASSIVE; //Suitable for binding a listening socket
	hints.ai_family = AF_UNSPEC; //getaddrinfo() will return address for any family
	hints.ai_socktype = SOCK_STREAM;
	if (getaddrinfo("127.0.0.1", "8080", &hints, &addr))
		return (-1);
	passive_sock = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
	if (passive_sock < 0)
		return (-1);
	setsockopt(passive_sock, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));
	if (bind(passive_sock, addr->ai_addr, addr->ai_addrlen) || listen(passive_sock, 10))
	{
		close(passive_sock);
		return (-1);
	}
	return (passive_sock);
}

int	accept_new_client(t_client*	clients, int epollfd, int passive_sock)
{
	int	i = 0;
	struct epoll_event	ev;

	while (i < NBR_CLIENTS && clients[i].fd >= 0)
		i++;
	if (i == NBR_CLIENTS)
		return (printf("Maximum number of clients reach"), 0);
	clients[i].fd = accept(passive_sock, &clients[i].addr, &clients[i].addr_size);
	if (clients[i].fd < 0)
		error ("accept", clients, passive_sock);
	get_ip_str(&clients[i].addr, clients[i].addr_str, sizeof(clients[i].addr_str));
	inet_ntop(AF_INET, &((struct sockaddr_in *)&clients[i].addr)->sin_addr, clients[i].addr_str, 16);
	printf("accept was done\n");
	ev.data.fd = clients[i].fd;
	ev.events = EPOLLIN | EPOLLHUP;// | EPOLLET;
	// fcntl(client_sock[client_index], F_SETFL , fcntl(client_sock[client_index], F_GETFL, 0) | O_NONBLOCK);
	if (epoll_ctl(epollfd, EPOLL_CTL_ADD, clients[i].fd, &ev))
		error("epoll_ctl", clients, passive_sock);
	return (0);
}

int	connection_close_client(t_client* client)
{
	printf("Interruption asked by client (fd %d), %s", client->fd, client->addr_str);
	close(client->fd);
	return (0);
}

int	read_client(t_client* client)
{
	int		nread;
	char	buffer[1024] = {0} ;

	printf("Reading from client (fd %d), %s\n", client->fd, client->addr_str);
	nread = read(client->fd, buffer, 1023);
	if (nread < 0)
		return (nread);
	if (nread == 0) //Chrome doing chrome
	{
		printf("Received POLLIN with nothing to read, closing connection\n");
		close(client->fd);
	}
	buffer[nread] = '\0';
	printf("%d bytes were read : %s", nread, buffer);
	fflush(stdout);
	return(0);
}

int	event_loop(t_client* clients, int epollfd, int passive_sock)
{
	struct epoll_event events[10];
	int				nbr_events, index;

	while (1)
	{
		nbr_events = epoll_wait(epollfd, events, 10, 0);
		if (nbr_events == 0)
			continue;
		else if (nbr_events < 0)
			error("epoll_wait", clients, passive_sock);
		// printf("%d events : %d\n", nbr_events, events[0].events);
		for (int i = 0; i < nbr_events; i++)
		{
			if (events[i].data.fd == passive_sock)
			{
				if (accept_new_client(clients, epollfd, passive_sock))
					return (-1);
			}
			else
			{
				for (index = 0; index < NBR_CLIENTS && clients[index].fd != events[i].data.fd; i++);
				if (index == NBR_CLIENTS)
				{
					printf("event (%u) from unkown client (fd %d)\n", events[i].events, events[i].data.fd);
					error(NULL, clients, passive_sock);
				}
				if (events[i].events & EPOLLHUP)
					connection_close_client(&clients[index]);
				else if (events[i].events & EPOLLIN)
					if (read_client(&clients[index]) < 0)
						error("reading from client", clients, passive_sock);
			}
		}
	}
}

int	main(void)
{
	int	passive_sock = 0;
	t_client	clients[NBR_CLIENTS] = {0};
	int 	epollfd = 0;
	struct epoll_event ev;

	for (int i = 0; i < NBR_CLIENTS; i++)
		clients[i].fd = -1;

	passive_sock = create_listen_socket();
	if (passive_sock < 0)
		error("creating passive socket", NULL, 0);
	printf("Socket is listening...\n");

	epollfd = epoll_create(1);
	if (epollfd < 0)
		error("epoll", NULL, passive_sock);
	ev.events = EPOLLIN;
	ev.data.fd = passive_sock;
	if (epoll_ctl(epollfd, EPOLL_CTL_ADD, passive_sock, &ev))
		error("epoll_ctl", NULL, passive_sock);

	event_loop(clients, epollfd, passive_sock);

	close(passive_sock);
	for (int i = 0; i < NBR_CLIENTS; i++)
		if (clients[i].fd >= 0)
			close(clients[i].fd);
	return (0);
}
