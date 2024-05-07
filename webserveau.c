#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <netinet/ip.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/epoll.h>

int	error(char *str)
{
	perror(str);
	exit(1);
}

int	main(void)
{
	int	passive_sock = 0, client_sock = 0;
	struct sockaddr_in	local_addr = {.sin_family = AF_INET,
									.sin_addr = {inet_addr("127.0.0.1")},
									.sin_port = htons(1080)};
	struct sockaddr_in	client_addr;
	socklen_t			client_addr_size = sizeof(client_addr);
	int 	epollfd = 0;
	struct epoll_event ev, events[10];
	char	buffer[1024] = {0};
	int		ret = 0, nbr_events;


	passive_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (passive_sock < 0)
		error("socket");
	int i = 1;
	setsockopt(passive_sock, SOL_SOCKET, SO_REUSEADDR, &i, sizeof(i));
	if (bind(passive_sock, (struct sockaddr *)&local_addr, sizeof(local_addr)))
	{
		close(passive_sock);
		error("bind");
	}
	if (listen(passive_sock, 3))
		error("listen");
	printf("Socket is listening...\n");


	epollfd = epoll_create(1);
	if (epollfd < 0)
		error("epoll");
	ev.events = EPOLLIN;
	ev.data.fd = passive_sock;
	if (epoll_ctl(epollfd, EPOLL_CTL_ADD, passive_sock, &ev))
		error("epoll_ctl");
	ev.events = EPOLLIN;
	ev.data.fd = STDIN_FILENO;
	if (epoll_ctl(epollfd, EPOLL_CTL_ADD, STDIN_FILENO, &ev))
		error("epoll_ctl");

	while (1)
	{
		nbr_events = epoll_wait(epollfd, events, 10, 0);
		if (nbr_events == 0)
			continue;
		else if (nbr_events < 0)
			error("epoll_wait");
		for (int i = 0; i < nbr_events; i++)
		{
			if (events[i].data.fd == passive_sock)
			{
				client_sock = accept(passive_sock, (struct sockaddr *)&client_addr, &client_addr_size);
				if (client_sock < 0)
					error ("accept");
				printf("accept was done\n");
				ev.data.fd = client_sock;
				ev.events = EPOLLIN;
				if (epoll_ctl(epollfd, EPOLL_CTL_ADD, client_sock, &ev))
					error("epoll_ctl");
			}
			else if (events[i].data.fd == client_sock)
			{
				ret = read(client_sock, buffer, 1023);
				if (ret < 0)
					error("read");
				if (ret == 0) //Disconnected socket ??
					epoll_ctl(epollfd, EPOLL_CTL_DEL, client_sock, NULL);
				buffer[ret] = '\0';
				printf("%d bytes were read : %s", ret, buffer);
			}
			else if (events[i].data.fd == STDIN_FILENO)
			{
				ret = read(STDIN_FILENO, buffer, 1024);
				if (ret <= 0)
					error("read from stdin");
				if (write(client_sock, buffer, ret) < 0)
					error("writing to client");
			}
		}
	}
	close(passive_sock);
	close(client_sock);
	return (0);
}