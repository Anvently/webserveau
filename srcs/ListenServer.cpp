#include <ListenServer.hpp>
#include <Host.hpp>
#include <Client.hpp>
#include <Request.hpp>
#include <ILogger.hpp>
#include <UniqueValuesMapIterator.tpp>

//TODO : Not so sure how to handle socket creation and listening errors ?
std::list<ListenServer>	ListenServer::_serverList;

ListenServer::ListenServer()
{
	_nbrHost = 0;
	_sockFd = 0;
	_maxClientNbr = MAX_CLIENT_NBR;
}

ListenServer::ListenServer(const ListenServer& copy) : _orphanClients(copy._orphanClients), \
	_connectedClients(copy._connectedClients), _sockFd(copy._sockFd), _hostMap(copy._hostMap), \
	_ip(copy._ip), _port(copy._port), _maxClientNbr(copy._maxClientNbr), _nbrHost(copy._nbrHost) {}

ListenServer::ListenServer(std::string const &hostAddr, std::string const &hostPort): _ip(hostAddr), _port(hostPort)
{
	_sockFd = 0;
	_nbrHost = 0;
	_maxClientNbr = MAX_CLIENT_NBR;
}

ListenServer::~ListenServer()
{
}

/// @brief Add host server_names as keys in server's ```_hostMap``` if they are
/// not already present.
/// @param host
void	ListenServer::assignHost(Host *host)
{
	for (std::vector<std::string>::const_iterator it = host->getServerNames().begin();\
			it != host->getServerNames().end(); it++)
	{
		if (_hostMap.find(*it) == _hostMap.end())
			_hostMap[*it] = host;

	}
}

// either assign the host to an existing ListenServer or create a new server with a socket
int	ListenServer::addHost(Host *host)
{
	std::list<ListenServer>::iterator	it = findServer(host->getAddr(), host->getPort());
	if (it != _serverList.end()) {
		it->assignHost(host);
		it->_nbrHost++;
	}
	else
	{
		ListenServer	*new_server = addServer(host->getAddr(), host->getPort());
		if (new_server == NULL)
			return (1);
		new_server->assignHost(host);
		new_server->_nbrHost++;
	}
	return (0);
}

std::list<ListenServer>::iterator	ListenServer::findServer(const std::string& hostAddr, const std::string& hostPort)
{
	std::list<ListenServer>::iterator	it;

	for (it = _serverList.begin(); it != _serverList.end(); it++)
	{
		if ((it->_ip == hostAddr) && (it->_port == hostPort))
			break;
	}
	return (it);
}

//Create a new ListenServer and a new socket listening to the given port, add the server to the static list
ListenServer*	ListenServer::addServer(const std::string& hostAddr, const std::string& hostPort)
{
	int				new_socket = 0, val = 1;
	struct	addrinfo *res, hints;
	memset(&hints, 0, sizeof(hints));
	hints.ai_flags = AI_PASSIVE;
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	if (getaddrinfo(NULL, hostPort.c_str(), &hints, &res))
		return (NULL);
	new_socket = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if (new_socket < 0) {
		freeaddrinfo(res);
		return (NULL);
	}
	setsockopt(new_socket, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));
	if (bind(new_socket, res->ai_addr, res->ai_addrlen))
	{
		LOGE("Could not create a socket for host: %ss and port %ss\n", &hostAddr, &hostPort);
		close(new_socket);
		freeaddrinfo(res);
		return (NULL);
	}
	ListenServer	serverTmp(hostAddr, hostPort);
	serverTmp._sockFd = new_socket;
	ListenServer&	newServer = *_serverList.insert(_serverList.end(), serverTmp);
	newServer._sockFd = new_socket;
	freeaddrinfo(res);
	return (&newServer);
}

void	ListenServer::removeServer(const std::string& hostAddr, const std::string& hostPort)
{
	std::list<ListenServer>::iterator	it = findServer(hostAddr, hostPort);
	if (it != _serverList.end())
		removeServer(it);
}

void	ListenServer::removeServer(std::list<ListenServer>::iterator it)
{
	if (it == _serverList.end())
		return;
	if (it->_sockFd)
		it->shutdown();
	for (std::list<Client*>::iterator clientIt = it->_orphanClients.begin(); clientIt != it->_orphanClients.end(); clientIt++)
	{
		Client::deleteClient(*clientIt);
		it->_orphanClients.remove(*clientIt);
	}
	for (UniqueValuesMapIterator<std::string, Host*> hostIt = it->_hostMap.begin(); hostIt != it->_hostMap.end();)
	{
		LOGD("Host (%ss) was unassigned from %ss:%ss", &hostIt->second->getServerNames().at(0), &it->_ip, &it->_port);
		it->unassignHost((hostIt++)->second);
	}
	_serverList.erase(it);
}

/// @brief Remove the host from the listen server it belongs to if any.
/// After the deletion, if the listen server doesn't refer to any host anymore,
/// it is terminated and deleted.
/// @param host
void	ListenServer::removeHost(Host* host) {
	std::list<ListenServer>::iterator it = findServer(host->getAddr(), host->getPort());
	if (it == _serverList.end())
		return;
	it->unassignHost(host);
	it->_nbrHost--;
	if (it->_hostMap.size() == 0)
		removeServer(it->_ip, it->_port);
}

/// @brief Unregister the host names from ```_hostMap```, delete the host.
/// Remove server if the host is the last one the server was refering to.
/// @param host
void	ListenServer::unassignHost(Host* host) {
	std::map<std::string, Host*>::iterator	mapIt;
	mapIt = _hostMap.find(host->getServerNames().at(0)); 
	if (mapIt == _hostMap.end())
		return;
	for (std::list<Client*>::const_iterator clientIt = mapIt->second->getClientListBegin();\
			clientIt != mapIt->second->getClientListEnd(); clientIt++)
		_connectedClients.remove(*clientIt);
	for (std::vector<std::string>::const_iterator nameIt = host->getServerNames().begin();\
			nameIt != host->getServerNames().end(); nameIt++)
	{
		mapIt = _hostMap.find(*nameIt);
		if (mapIt != _hostMap.end())
			_hostMap.erase(mapIt);
	}
	Host::removeHost(host);
}

// add the socket to the epoll interest list with a pointer to ListenServer in event.data. Then tells the socket to listen.
int	ListenServer::registerToEpoll(int epollfd)
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

int	ListenServer::getNbrConnectedClients(void) const {
	return (this->_connectedClients.size());
}

void	ListenServer::shutdown()
{
	close(this->_sockFd);
	this->_sockFd = 0;
}


// Start all the servers using the start function.
int	ListenServer::startServers(int epollfd)
{
	for (std::list<ListenServer>::iterator it = _serverList.begin(); it != _serverList.end(); it++)
	{
		if (it->registerToEpoll(epollfd))
		{
			_serverList.erase(it);
			LOGE("Could not listen on host:%s port:%s\n", it->_ip.c_str(), it->_port.c_str());
		}
		else {
			LOGI("Listening on host:%s port:%s\n", it->_ip.c_str(), it->_port.c_str());
			LOGI("%Ls", &*it);
		}
	}
	return (0);
}

//Not sure it is needed since the destructor should close the socketfd anyway
void	ListenServer::closeServers()
{
	for (std::list<ListenServer>::iterator it = _serverList.begin(); it != _serverList.end(); it++){
		it->shutdown();
	}
}

void	ListenServer::deleteServers()
{
	for (std::list<ListenServer>::iterator it = _serverList.begin(); it != _serverList.end();)
	{
		removeServer(it++);
	}
}

int	ListenServer::getNbrServer(void) {
	return (_serverList.size());
}

int ListenServer::getNbrHost(void) const {
	return (this->_nbrHost);
}

/// @brief Accept an incoming connection, create a client and add it to the
/// server's orphan list. MUST follows an epoll event insuring that the call
/// to ```accept()``` will not block. Client will then have to be assign externally
/// to its correct host when header parsing is done.
/// @param
/// @return ```NULL``` if no client could be initiated or if connection
/// socket could not be created.
Client*	ListenServer::acceptConnection(void) {
	if (getNbrConnectedClients() >= _maxClientNbr) {
		close(accept(_sockFd, NULL, NULL));
		LOGE("A connection was refused by server (%ss:%ss) because the maximum" \
			" number of allowed clients was reached.", &_ip, &_port);
		return (NULL);
	}
	ClientSocket	socket;
	socket.addrSize = sizeof(socket.addr);
	socket.fd = accept(_sockFd, &socket.addr, &socket.addrSize);
	if (socket.fd < 0) {
		LOGE("Server %ss:%ss failed to accept a new client.", &_ip, &_port);
		return (NULL);
	}
	Client*	newClient = Client::newClient(socket, *this);
	if (newClient == NULL) {
		LOGE("Unexpected error when creating new client");
		close(socket.fd);
		return (NULL);
	}
	_connectedClients.push_back(newClient);
	_orphanClients.push_back(newClient);
	LOGI("A new client (%ss) connected to server %ss:%ss", &newClient->getStrAddr(), &_ip, &_port);
	return (newClient);
}

/// @brief Try to bind an orphan client to an host using parsed host header.
/// If the host is not referred by the listen server, it will try to bind the client
/// to the first ```server_name``` of the list. hostName should not be empty as
/// the empty ```host``` header situation should be handled by ```IControl```.
/// Client host is not set.
/// @param hostName
/// @return ```NULL``` if given hostName was not found in the host list of the listen server.
Host*	ListenServer::bindClient(Client& client, const std::string& hostName)
{
	std::map<std::string, Host*>::iterator	it;

	it = _hostMap.find(hostName);
	if (it == _hostMap.end())
		it = _hostMap.begin();
	it->second->addClient(&client);
	_orphanClients.remove(&client);
	return (it->second);
}

std::ostream&	ListenServer::printShort(std::ostream& os) const {
	os << "	->  ip/port: " << this->_ip << '/' << this->_port << std::endl;
	os << "	sock_fd: " << this->_sockFd << std::endl;
	os << "	number of connected clients (orphan): " << this->_connectedClients.size() \
		<< " (" << this->_orphanClients.size() << ")\n";
	os << "	host list (" << this->getNbrHost() << "):\n";
	for (UniqueValuesMapIterator_const<std::string, Host*> it(this->_hostMap.begin()); it != this->_hostMap.end(); it++)
		it->second->printShort(os);
	return (os);
}

std::ostream&	ListenServer::printFull(std::ostream& os) const {
	os << "	->  ip/port: " << this->_ip << '/' << this->_port << std::endl;
	os << "	sock_fd: " << this->_sockFd << std::endl;
	os << "	number of connected clients (orphan): " << this->_connectedClients.size() \
		<< " (" << this->_orphanClients.size() << ")\n";
	os << "	host list (" << this->getNbrHost() << "):\n";
	for (UniqueValuesMapIterator_const<std::string, Host*> it(this->_hostMap.begin()); it != this->_hostMap.end(); it++)
		it->second->printFull(os);
	return (os);
}

std::ostream&	operator<<(std::ostream& os, const ListenServer& ls) {
	ls.printShort(os);
	return (os);
}
