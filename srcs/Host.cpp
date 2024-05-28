#include <Host.hpp>
#include <sstream>
#include <algorithm>
#include <limits.h>

std::list<Host>			Host::_hostList;
std::vector<CGIConfig>	Host::_cgis;
std::vector<Location>	Host::_locations;

Host::~Host(void) {}

Host::Host(void): _body_max_size(INT_MAX)
{}

Host&	Host::operator=(Host& rhs) {
	(void) rhs;
	return (*this);
}

Host::Host(const Host& copy) : _addr(copy._addr), _port(copy._port), \
	_body_max_size(copy._body_max_size), _dir_errors(copy._dir_errors), \
	_server_names(copy._server_names), _locationMap(copy._locationMap), \
	_cgiMap(copy._cgiMap), _clients(copy._clients) {}

Location::Location(void) : dir_listing(false), upload(false) {
	for (int i = 0; i < METHOD_NBR; i++) {
		methods[i] = false;
	}
}

Location::Location(const Location& copy) : root(copy.root), dir_listing(copy.dir_listing), \
	default_uri(copy.default_uri), upload(copy.upload), upload_root(copy.upload_root), \
	addr_redir(copy.addr_redir)
{
	for (int i = 0; i < METHOD_NBR; i++) {
		methods[i] = copy.methods[i];
	}
}

CGIConfig::CGIConfig(void) {
	for (int i = 0; i < METHOD_NBR; i++) {
		methods[i] = false;
	}
}

CGIConfig::CGIConfig(const CGIConfig& copy) : exec(copy.exec), root(copy.root)
{
	for (int i = 0; i < METHOD_NBR; i++) {
		methods[i] = copy.methods[i];
	}
}

std::list<Host>::iterator	Host::findHost(Host* host)
{
	std::list<Host>::iterator pos;
	for (pos = _hostList.begin(); pos != _hostList.end(); pos++)
	{
		if (host == &*pos)
			return (pos);
	}
	return (pos);
}

/// @brief Append a h
/// @param host
/// @brief Append a host to the list of host and call
/// addHost on ListenServer
/// @param host
void	Host::addHost(Host& host) {
	Host& newHost = *_hostList.insert(_hostList.end(), host);
	ListenServer::addHost(&newHost);
}

void	Host::removeHost(Host* host) {
	if (host == NULL)
		return;
	host->shutdown();
	std::list<Host>::iterator pos = findHost(host);
	if (pos != _hostList.end())
		_hostList.erase(pos);
}

const std::string&	Host::getPort(void) const {
	return (this->_port);
}

int	Host::getMaxSize(void) const {
	return (this->_body_max_size);
}

const std::string&	Host::getAddr(void) const {
	return (this->_addr);
}

Client*	Host::getClientByFd(int fd) const {
	(void) fd;
	return (NULL);
}

Location*	Host::getLocation(const std::string& path) {
	std::map<std::string, Location*>::iterator	pos = _locationMap.find(path);
	if (pos == _locationMap.end())
		return (NULL);
	return (pos->second);
}

const std::vector<std::string>&	Host::getServerNames(void) const {
	return (this->_server_names);
}

std::list<Client*>::const_iterator	Host::getClientListBegin(void) const {
	return (_clients.begin());
}

std::list<Client*>::const_iterator	Host::getClientListEnd(void) const {
	return (_clients.end());
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

/// @brief Terminate and close connection with every clients belonging to the host.
/// @param
void	Host::shutdown(void) {
	for (std::list<Client*>::iterator it = _clients.begin(); it != _clients.end(); it++)
		Client::deleteClient(*it);
}

bool	CGIConfig::operator==(const CGIConfig& rhs) const {
	if (this->exec != rhs.exec
		|| this->root != rhs.root
		|| !std::equal(&rhs.methods[0], &rhs.methods[METHOD_NBR], &this->methods[0]))
		return (false);
	return (true);
}

bool	Location::operator==(const Location& rhs) const {
	if (!std::equal(rhs.addr_redir.begin(), rhs.addr_redir.end(), this->addr_redir.begin())
		|| this->default_uri != rhs.default_uri
		|| this->dir_listing != rhs.dir_listing
		|| !std::equal(&rhs.methods[0], &rhs.methods[METHOD_NBR], &this->methods[0])
		|| this->upload != rhs.upload
		|| this->upload_root != rhs.upload_root)
		return (false);
	return (true);
}

void	Host::addCGIConfig(const std::deque<std::string>& names, CGIConfig& cgiConfig)
{
	std::vector<CGIConfig>::iterator pos;

	pos = std::find(_cgis.begin(), _cgis.end(), cgiConfig);
	if (pos == _cgis.end())
		_cgis.insert(pos, cgiConfig);
	for (std::deque<std::string>::const_iterator it = names.begin(); it != names.end(); it++)
		_cgiMap[*it] = &*pos;
}

void	Host::addLocation(const std::deque<std::string>& names, Location& location)
{
	std::vector<Location>::iterator pos;

	pos = std::find(_locations.begin(), _locations.end(), location);
	if (pos == _locations.end())
		_locations.insert(pos, location);
	for (std::deque<std::string>::const_iterator it = names.begin(); it != names.end(); it++)
		_locationMap[*it] = &*pos;
}

void	Host::printProperties(std::ostream& os) const
{
	os << "		->  Names :";
	for (std::vector<std::string>::const_iterator it = _server_names.begin(); \
		it != _server_names.end(); it++)
		os << " " << *it;
	os << std::endl;
	os << "	Max body size : " << _body_max_size << std::endl;
	os << "	Error dir : " << _dir_errors << std::endl;

}

void	Host::printShort(std::ostream& os) const
{
	printProperties(os);
	os << "		Locations :";
	for (std::map<std::string, Location*>::const_iterator it = _locationMap.begin();\
			it != _locationMap.end(); it++)
		os << " " << it->first;
	os << std::endl;
	os << "		CGIs :";
	for (std::map<std::string, CGIConfig*>::const_iterator it = _cgiMap.begin();\
			it != _cgiMap.end(); it++)
		os << " " << it->first;
	os << std::endl;
	os << "		Clients (" << _clients.size() << ") :";
	// for (std::list<Client*>::const_iterator it = _clients.begin(); it != _clients.end(); it++)
		// os << " " << (*it ? (**it).getfd() : -2);
	os << std::endl;
}

void	Host::printFull(std::ostream& os) const
{
	printProperties(os);
	os << "		Locations :" << std::endl;
	for (std::map<std::string, Location*>::const_iterator it = _locationMap.begin();\
			it != _locationMap.end(); it++)
		os << "	  ->" << it->first << std::endl << it->second;
	os << "		CGIs :";
	for (std::map<std::string, CGIConfig*>::const_iterator it = _cgiMap.begin();\
			it != _cgiMap.end(); it++)
		os << "		  ->" << it->first << std::endl << it->second;
	os << "		Clients (" << _clients.size() << ") :" << std::endl;
	// for (std::list<Client*>::const_iterator it = _clients.begin(); it != _clients.end(); it++)
	// {
		// if (*it)
			// os << "	  ->" << (**it).getfd() << std::endl << **it;
		// else
			// os << "	  => (null)" << std::endl;
	// }
	os << std::endl;
}

std::ostream&	operator<<(std::ostream& os, const Location& location)
{
	os << "			->  root: " << location.root << std::endl;
	os << "			default_uri: " << location.default_uri << std::endl;
	os << "			dir_listing: " << location.dir_listing << std::endl;
	os << "			upload: " << location.upload << std::endl;
	if (location.upload)
		os << "			upload_root: " << location.upload_root << std::endl;
	os << "			allowed methods: ";
	for (int i = 0; i < METHOD_NBR; i++)
		os << (location.methods[i] ? METHOD_STR[i] : "");
	os << std::endl;
	os << "			redirections: \n";
	for (std::map<int, std::string>::const_iterator it = location.addr_redir.begin();\
			it != location.addr_redir.end(); it++)
		os << "			  - " << it->first << " => " << it->second << std::endl;
	return (os);
}

std::ostream&	operator<<(std::ostream& os, const CGIConfig& CGIConfig)
{
	os << "			->  root: " << CGIConfig.root << std::endl;
	os << "			exec_path: " << CGIConfig.exec << std::endl;
	os << "			allowed methods: ";
	for (int i = 0; i < METHOD_NBR; i++)
		os << (CGIConfig.methods[i] ? METHOD_STR[i] : "");
	os << std::endl;
	return (os);
}
std::ostream&	operator<<(std::ostream& os, const Host& host)
{
	host.printShort(os);
	return (os);
}
