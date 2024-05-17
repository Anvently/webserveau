#include <ListenServer.hpp>


//TODO : Not so sure how to handle socket creation and listening errors ?


void	ListenServer::pushHost(Host *host)
{
	this->_hostList.push_back(host);
}

// either assign the host to an existing ListenServer or create a new server with a socket

int	ListenServer::addHost(Host *host)
{
	std::list<ListenServer>::iterator	it;

	for (std::list<ListenServer>::iterator it = serverList.begin(); it != serverList.end(); it++)
	{
		if ((it->_ip == host->getHost()) && (it->_port == host->getPort()))
			break;
	}
	if (it != serverList.end())
		it->pushHost(host);
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
	close(this->_sockFd); // Not sure it is needed?
}



bool	ListenServer::isMatch(std::string const &hostAddr, std::string const &hostPort)
{
	return (this->_ip == hostAddr && this->_port == hostPort);
}

//Create a new ListenServer and a new socket listening to the given port, add the server to the static list

ListenServer*	ListenServer::addServer(const std::string& hostAddr, const std::string& hostPort)
{
	ListenServer	new_server(hostAddr, hostPort);
	int				new_socket = 0, val = 1;
	struct	addrinfo *res, hints;
	memset(&hints, 0, sizeof(hints));
	hints.ai_flags = AI_PASSIVE;
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	if (getaddrinfo(NULL, hostPort.c_str(), &hints, &res))
		return (NULL);
	new_socket = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if (new_socket < 0)
		return (NULL);
	setsockopt(new_socket, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));
	if (bind(new_socket, res->ai_addr, res->ai_addrlen))
	{
		LOGE("Could not create a socket for host: %s and port %s\n", hostAddr.c_str(), hostPort.c_str());
		close(new_socket);
		return (NULL);
	}
	new_server._sockFd = new_socket;
	serverList.push_back(new_server);
	return (&new_server);
}

// add the socket to the epoll interest list with a pointer to ListenServer in event.data. Then tells the socket to listen.

int	ListenServer::start(int epollfd)
{
	struct epoll_event	event;

	event.events = EPOLLIN;
	event.data.ptr = this;
	if (epoll_ctl(epollfd, EPOLL_CTL_ADD, this->_sockFd, &event))
	{
		close(this->_sockFd);
		return (-1);
	}
	if (listen(this->_sockFd, 4096))
	{
		close(this->_sockFd);
		return (-1);
	}
	return (0);
}


int	ListenServer::terminate()
{
	close(this->_sockFd);
	return (0);
}


// Start all the servers using the start function.

int	ListenServer::startServers(int epollfd)
{
	for (std::list<ListenServer>::iterator it = serverList.begin(); it != serverList.end(); it++)
	{
		if (it->start(epollfd))
		{
			serverList.erase(it);
			LOGE("Could not listen on host:%s port:%s\n", it->_ip.c_str(), it->_port.c_str());
		}
		else
			LOGI("Listening on host:%s port:%s\n", it->_ip.c_str(), it->_port.c_str());
	}
	return (0);
}


//Not sure it is needed since the destructor should close the socketfd anyway
int	ListenServer::closeServers()
{
	for (std::list<ListenServer>::iterator it = serverList.begin(); it != serverList.end(); it++){
		it->terminate();
	}
	return (0);
}
