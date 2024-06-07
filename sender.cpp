#include <fstream>
#include <sys/socket.h>
#include <iostream>
#include <unistd.h>
#include <string>
#include <sys/epoll.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 15
#define PAUSE_TIME 500000
#define ADDRESS "127.0.0.1"
#define PORT 8080

int	error(const char* context) {
	std::cout << context << std::endl;
	return (1);
}

int	sendFile(std::ifstream& infile, int sock) {
	char	buffer[BUFFER_SIZE];
	int		nread, nwrite;

	nread = infile.readsome(buffer, BUFFER_SIZE);
	if (nread <= 0)
		return (-1);
	nwrite = write(sock, buffer, nread);
	if (nwrite < 0) {
		perror("");
		// recv(sock, NULL, 10, 0);
		return (error("write error"));
	}
	else if (nwrite != nread)
		std::cout << "warning: number of written characters is different" \
			"from number of read characters" << std::endl;
	else if (infile.eof())
		return (-1);
	else if (infile.bad())
		return (error("infile error"));
	usleep(PAUSE_TIME);
	return (0);
}

int	readSock(int sock) {
	char	buffer[BUFFER_SIZE];
	int		nread = 0;
	if ((nread = read(sock, buffer, BUFFER_SIZE - 1)) < 0)
		return (error("reading socket"));
	else if (nread == 0)
		return (-1);
	buffer[nread] = '\0';
	printf("%s", buffer);
	return (0);
}

int	openFile(std::ifstream& infile,  char* path) {
	infile.open(path);
	if (infile.bad())
		return (1);
	return (0);
}

int	connectSock(int* sock) {
	struct sockaddr_in	addr;
	
	addr.sin_family = AF_INET;
	addr.sin_port = htons(PORT);
	if (inet_pton(AF_INET, ADDRESS, &addr.sin_addr) <= 0)
		return (error("invalid address"));
	*sock = socket(AF_INET, SOCK_STREAM, 0);
	if (*sock < 0)
		return (error("Failed to create socket"));
	if (connect(*sock, (struct 	sockaddr*)&addr, sizeof(addr)) < 0)
		return (error("failed to connect"));
	return (0);
}

int	registerToEpoll(int epollfd, int fd, int flags) {
	struct epoll_event	event;

	event.events = flags;
	event.data.fd = fd;
	if (epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event))
		return (-1);
	return (0);
}

int	changeEpoll(int epollfd, int fd, int flags) {
	struct epoll_event	event;

	event.events = flags;
	event.data.fd = fd;
	if (epoll_ctl(epollfd, EPOLL_CTL_MOD, fd, &event))
		return (-1);
	return (0);
}

int	epoll_loop(int epollfd, int sock, std::ifstream& infile) {
	struct	epoll_event	events[10];
	int	nbr_events = 0;

	while ((nbr_events = epoll_wait(epollfd, events, 10, -1)) >= 0) {
		for (int i = 0; i < nbr_events; i++) {
			if (events[i].events & EPOLLOUT) {
				switch (sendFile(infile, sock))
				{
					case -1:
						if (changeEpoll(epollfd, sock, EPOLLIN | EPOLLHUP))
							return (1);
						break;

					case 1:
						return (1);
				}
			}
			if (events[i].events & EPOLLIN) {
				switch (readSock(sock))
				{
					case -1:
						printf("Read 0 characters. Closing connection\n");
						close(sock);
						return (0);

					case 1:
						return (1);
				}
			}
			if (events[i].events & EPOLLHUP) {
				close(sock);
				printf("EPOLLHUP !");
				return (0);
			}
		}
	}
	return (nbr_events);
}

int	main(int argc, char** argv) {
	int				sock = -1, epollfd = -1, res;
	std::ifstream	infile;
	if (argc != 2)
		return (error("invalid number of arguments"));
	if (openFile(infile, argv[1]))
		return (error("failed to open file"));
	if (connectSock(&sock))
		return (1);
	if ((epollfd = epoll_create(1)) < 0)
		return (error("failed to create epoll"));
	if (registerToEpoll(epollfd, sock, EPOLLIN | EPOLLOUT | EPOLLHUP))
		return (error("failed to register to epollout"));
	if (epoll_loop(epollfd, sock, infile))
		return (error("error in loop"));
	close (epollfd);
	return (0);
}