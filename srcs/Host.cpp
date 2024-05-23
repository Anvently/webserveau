#include <Host.hpp>
#include <sstream>
#include <algorithm>

std::list<Host>		Host::_hostList;

Host::~Host(void) {}

Host::Host(void) {}

Host&	Host::operator=(Host& rhs) {
	(void) rhs;
	return (*this);
}

Host::Host(const Host& copy) {
	(void) copy;
}

void	Host::addHost(Host& host) {
	_hostList.push_back(host);
}

int	Host::getPort(void) const {
	return (this->_port);
}

int	Host::getMaxSize(void) const {
	return (this->_client_max_size);
}

std::string	Host::getHost(void) const {
	std::stringstream	stream;
	stream << this->_name << ':' << this->_port;
	return (stream.str());
}

Client*	Host::getClientByFd(int fd) const {
	(void) fd;
	return (NULL);
}

Location*	Host::getLocation(const std::string& path) {
	std::map<std::string, Location>::iterator	pos = _locations.find(path);
	if (pos == _locations.end())
		return (NULL);
	return (&pos->second);
}

bool	Host::checkServerName(const std::string& name) const {
	std::vector<std::string>::const_iterator pos;
	pos = std::find(_server_names.begin(), _server_names.end(), name);
	if (pos == _server_names.end())
		return (false);
	return (true);
}

void	Host::removeClient(Client* client) {
	if (client)
		this->_clients.remove(client);
}

void	Host::addClient(Client* newClient) {
	if (newClient)
		this->_clients.push_back(newClient);
}
