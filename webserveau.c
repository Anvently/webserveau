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

int	error(char *str)
{
	printf("EXIT\n");
	perror(str);
	exit(1);
}

int	main(void)
{
	int	passive_sock = 0, client_sock[10] = {0}, client_index = 0;
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
	// ev.events = EPOLLIN;
	// ev.data.fd = STDIN_FILENO;
	// if (epoll_ctl(epollfd, EPOLL_CTL_ADD, STDIN_FILENO, &ev))
	// 	error("epoll_ctl");

	while (1)
	{
		nbr_events = epoll_wait(epollfd, events, 10, 0);
		if (nbr_events == 0)
			continue;
		else if (nbr_events < 0)
			error("epoll_wait");
		printf("%d events occured\n", nbr_events);
		for (int i = 0; i < nbr_events; i++)
		{
			// printf("client sock = %d | passive sock = %d\n", client_sock, passive_sock);
			// printf("fd = %d | event = %d\n", events[i].data.fd, events[i].events);
			if (events[i].data.fd == passive_sock)
			{
				client_sock[client_index] = accept(passive_sock, (struct sockaddr *)&client_addr, &client_addr_size);
				if (client_sock[client_index] < 0)
					error ("accept");
				printf("accept was done\n");
				ev.data.fd = client_sock[client_index];
				ev.events = EPOLLIN | EPOLLHUP;// | EPOLLET;
				// fcntl(client_sock[client_index], F_SETFL , fcntl(client_sock[client_index], F_GETFL, 0) | O_NONBLOCK);
				if (epoll_ctl(epollfd, EPOLL_CTL_ADD, client_sock[client_index], &ev))
					error("epoll_ctl");
				client_index++;
			}
			else
			{
				// printf("event = %u\n", events[i].events);
				if (events[i].events & EPOLLHUP)
				{
					printf("Interruption asked by client");
					close(events[i].data.fd);
				}
				else if (events[i].events & EPOLLIN)
				{
					ret = read(events[i].data.fd, buffer, 1023);
					if (ret < 0)
						error("read");
					if (ret == 0) //Chrome doing chrome
					{
						printf("Received POLLIN with nothing to read\n");
						close(events[i].data.fd);
						continue;
					}
					buffer[ret] = '\0';
					printf("%d bytes were read : %s", ret, buffer);
					// int	fd = open("response", O_RDONLY);
					// int	nread = 0;
					// char	str[1024];
					// while ((nread = read(fd, str, 1023)) > 0)
					// 	write (events[i].data.fd, str, nread);
					// // if (write(events[i].data.fd, "\0", 1) < 0)
					// // 	error("writing null byte");
					// close(fd);
				}
			}
		}
	}
	close(passive_sock);
	for (int i = 0; i < client_index; i++)
		close(client_sock[i]);
	return (0);
}
