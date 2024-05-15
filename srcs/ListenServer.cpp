#include <ListenServer.hpp>




void	ListenServer::pushHost(Host *host)
{
	this->_hostList.push_back(host);
}

int	ListenServer::addHost(Host *host)
{
	std::list<ListenServer>::iterator	it;

	it = std::find_if(serverList.begin(), serverList.end(), [](ListenServer inst){return (inst.isMatch(host->getHost(), host->getPort()))});
	if (it != serverList.end())
		it.pushHost(host);
	else
	{
		ListenServer	*new_server = addServer(host->getHost(), host->getPort());
		if (new_server == 0)
			return (1);
		new_server->pushHost(host);
	}
	return (0);
}


ListenServer::ListenServer(): _sockFd(0), _hostList()
{

}

ListenServer::ListenServer(std::string const &hostAddr, std::string const &hostPort): _ip(hostAddr), _port(hostPort)
{

}

ListenServer::~ListenServer()
{
	close(this->_sockFd);
}

bool	ListenServer::isMatch(std::string const &hostAddr, std::string const &hostPort)
{
	return (this->_ip == hostAddr && this->_port == hostPort);
}

ListenServer*	ListenServer::addServer(const std::string& hostAddr, const std::string& hostPort)
{
	ListenServer	new_server(hostAddr, hostPort);
	int				new_socket = 0, val = 1;
	struct	addrinfo *res, hints;
	memset(&hints, 0, sizeof(hints));
	hints.ai_flags = AI_PASSIVE;
	hints.ai_AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	if (getaddrinfo(NULL, hostPort, &hints, &res))
		return (NULL);
	new_socket = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if (new_socket < 0)
		return (NULL);
	setsockopt(new_socket, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));
	if (bind(new_socket, res->ai_addr, res->ai_addrlen))
	{
		LOGE("Could not create a socket for host: %s and port %s\n", hostAddr, hostPort);
		close(new_socket);
		return (1);
	}
	new_server._sockFd = new_socket;
	serverList.push_back(new_server);
	return (&new_server);
}


int	ListenServer::start(int epollfd)
{
	struct epoll_event	event;

	event.events = EPOLLIN;
	ev.data.ptr = this;
	if (epoll_ctl(epollfd, EPOLL_CTL_ADD, this->_sockFd, &event))
	{
		LOGE("epoll_ctl error on host %s and port %s\n", this->_ip, this->_port);
		return (-1);
	}
	if (listen(this->_sockFd, 4096))
	{
		LOGE("Could not listen on socket %d associated with host %s and port %s\n", this->_sockFd, this->_ip, this->_port);
		return (-1);
	}
	return (0);
}


int	ListenServer::close()
{
	close(this->_sockFd);
	return (0);
}

int	ListenServer::startServers(int epollfd)
{
	for (std::list<ListenServer>::iterator it = serverList.begin(); it != serverList.end(); it++)
	{
		it->start(epollfd)
	}
	return (0);
}

int	ListenServer::closeServers()
{
	for (std::list<ListenServer>::iterator it = serverList.begin(); it != serverList.end(); it++){
		it->close();
	}
	return (0);
}
