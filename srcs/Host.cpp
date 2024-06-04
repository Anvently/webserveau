#include <Host.hpp>
#include <sstream>
#include <algorithm>
#include <ILogger.hpp>
#include <limits.h>

std::list<Host>			Host::_hostList;
std::vector<CGIConfig>	Host::_cgis;
std::vector<Location>	Host::_locations;

Host::~Host(void) {}

Host::Host(void): _body_max_size(INT_MAX)
{}

Host&	Host::operator=(Host& rhs) {
	(void) rhs;
	LOGE("Undefined assigment operator for host class");
	return (*this);
}

Host::Host(const Host& copy) : _addr(copy._addr), _port(copy._port), \
	_body_max_size(copy._body_max_size), _dir_errors(copy._dir_errors), \
	_server_names(copy._server_names), _locationMap(copy._locationMap), \
	_cgiMap(copy._cgiMap), _clients(copy._clients) {}

Location::Location(void) : methods(GET), dir_listing(false), upload(false), redir(0) {}

Location::Location(const Location& copy) : root(copy.root), methods(copy.methods), dir_listing(copy.dir_listing), \
	default_uri(copy.default_uri), upload(copy.upload), upload_root(copy.upload_root), \
	redir(copy.redir), addr_redir(copy.addr_redir)
{}

CGIConfig::CGIConfig(void) : methods(GET) {}

CGIConfig::CGIConfig(const CGIConfig& copy) : exec(copy.exec), root(copy.root), \
methods(copy.methods), extension(copy.extension)
{}

bool	CGIConfig::operator==(const CGIConfig& rhs) const {
	if (this->exec != rhs.exec
		|| this->root != rhs.root
		|| this->methods != rhs.methods)
		return (false);
	return (true);
}

bool	Location::operator==(const Location& rhs) const {
	if (this->redir == rhs.redir
		|| !std::equal(rhs.addr_redir.begin(), rhs.addr_redir.end(), this->addr_redir.begin())
		|| this->default_uri != rhs.default_uri
		|| this->dir_listing != rhs.dir_listing
		|| this->methods != rhs.methods
		|| this->upload != rhs.upload
		|| this->upload_root != rhs.upload_root)
		return (false);
	return (true);
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

const Location*	Host::getLocation(const std::string& uri) const {
	return (getMapObjectByKey(_locationMap, uri));
}

const CGIConfig*	Host::getCGIConfig(const std::string& uri) const {
	return (getMapObjectByKey(_cgiMap, uri));
}

const Location*	Host::matchLocation(const std::string& uri) const {
	return (getObjectMatch(_locationMap, uri));
}

const CGIConfig*	Host::matchCGIConfig(const std::string& uri) const {
	return (getObjectMatch(_cgiMap, uri));
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
		os << ((location.methods & (1 << i)) ? METHOD_STR[i] : "");
	os << std::endl;
	if (location.redir) {
		os << "			redirections (" <<  location.redir << "): \n";
		for (std::vector<std::string>::const_iterator it = location.addr_redir.begin();\
				it != location.addr_redir.end(); it++)
			os << "			  - " << *it << std::endl;
	}
	return (os);
}

std::ostream&	operator<<(std::ostream& os, const CGIConfig& CGIConfig)
{
	os << "			->  root: " << CGIConfig.root << std::endl;
	os << "			exec_path: " << CGIConfig.exec << std::endl;
	os << "			allowed methods: ";
	for (int i = 0; i < METHOD_NBR; i++)
		os << ((CGIConfig.methods & (1 << i)) ? METHOD_STR[i] : "");
	os << std::endl;
	return (os);
}
std::ostream&	operator<<(std::ostream& os, const Host& host)
{
	host.printShort(os);
	return (os);
}

/// @brief Check if the location directory indicates a redirection.
/// Redirection are handled without additionnal checking 
/// @param client 
/// @param request 
/// @return 
int	Host::checkRedirection(Request& request) const
{
	const Location& location = *request._resHints.locationRules;
	switch (location.redir)
	{
		case 0:
			return (0);
	
		case RES_MULTIPLE_CHOICE:
			request._resHints.status = RES_MULTIPLE_CHOICE;
			break;

		case RES_MOVED_PERMANENTLY:
			request._resHints.status = RES_MOVED_PERMANENTLY;
			break;

		case RES_FOUND:
			request._resHints.status = RES_FOUND;
			break;

		case RES_SEE_OTHER:
			request._resHints.status = RES_SEE_OTHER;
			break;

		case RES_TEMPORARY_REDIRECT:
			request._resHints.status = RES_TEMPORARY_REDIRECT;
			break;

		default:
			LOGE("Invalid or unsupported redirection status (%d)", location.redir);
			break;
	}
	request._resHints.redirList = &location.addr_redir;
	return (0);
}

/// @brief Try to match the request to location/cgi rules and to a type
/// using its ```uri``` structure. Uri is parsed again using to determine
/// ```pathinfo``` in case of CGI. Return ```RES_NOT_FOUND``` if error (```resHints```
/// is updated).
/// @param request 
/// @return 
int	Host::matchRequest(Request& request) const
{
	request._resHints.locationRules = matchLocation(request._parsedUri.path);
	request._resHints.cgiRules = matchCGIConfig(request._parsedUri.path);
	if (request._resHints.locationRules == NULL && request._resHints.cgiRules == NULL) {
		request._resHints.status = RES_NOT_FOUND;
		request._type = REQ_TYPE_NO_MATCH;
		return (RES_NOT_FOUND);
	}
	else if (request._resHints.cgiRules) {
		request._type = REQ_TYPE_CGI;
		request.extractPathInfo(request._resHints.cgiRules->extension);
		if (request._parsedUri.pathInfo.find('/') != std::string::npos)
			request._resHints.locationRules = matchLocation(request._parsedUri.path);
	}
	else if (request._parsedUri.extension == "/")
		request._type = REQ_TYPE_DIR;
	else
		request._type = REQ_TYPE_STATIC;
	return (0);
}

/// Should be called if the ressource was a directory.
/// Switch path to index file if given by host. Request is then
/// matched again to determine its final type.
/// Return error if dir_listing is not allowed, if methods is not ```GET```
int	Host::checkDirRessource(Request& request) const
{
	const Location& location = *request._resHints.locationRules;
	if (location.default_uri != "") {
		request.parseURI(location.default_uri);
		if (matchRequest(request))
			return (RES_NOT_FOUND);
		return (0);
	} else if (location.dir_listing == true) {
		if (request._method != GET) {
			request._resHints.status = RES_FORBIDDEN;
			return (RES_FORBIDDEN);
		}
		request._resHints.path = request._parsedUri.path;
	}
	return (0);
}

/// @brief Check allowed methods for given location, and if upload is
/// allowed for static upload.
/// @param location 
/// @param request 
/// @return 
int	Host::checkLocationRules(Request& request) const
{
	if ((request._resHints.locationRules->methods & request._method) == 0) {
		request._resHints.status = RES_METHOD_NOT_ALLOWED;
		return (RES_METHOD_NOT_ALLOWED);
	}
	if (request._method == POST && request._type == REQ_TYPE_STATIC) {
		if (request._resHints.locationRules->upload == false) {
			request._resHints.status = RES_FORBIDDEN;
			return (RES_FORBIDDEN);
		}
	}
	return (0);
}

/// @brief Check if methods match allowed methods for CGI
/// @param cgi 
/// @param request 
/// @return 
int	Host::checkCGIRules(Request& request) const
{
	if ((request._resHints.cgiRules->methods & request._method) == 0) {
		request._resHints.status = RES_METHOD_NOT_ALLOWED;
		return (RES_METHOD_NOT_ALLOWED);
	}
	return (0);
}

/// @brief Check if the given path exist from the server perspective.
/// Symbolic link or other type of files are ignored.
/// @param path 
/// @param type Can be provided to make additionnal check regarding file type
/// ```REQ_TYPE_DIR``` vs ```REQ_TYPE_CGI||REQ_TYPE_STATIC```.
/// @return 
int	Host::checkRessourcePath(const std::string& path, int type)
{
	struct stat	f_stat;

	if (stat(path.c_str(), &f_stat))
		return (RES_NOT_FOUND);
	else if (type != REQ_TYPE_DIR && S_ISREG(f_stat.st_mode) == false)
		return (RES_FORBIDDEN);
	else if (type == REQ_TYPE_DIR && S_ISDIR(f_stat.st_mode) == false)
		return (RES_NOT_FOUND);
	return (0);
}

int	Host::checkRessourceExistence(Request& request) const {
	std::string	path;
	int			res;

	if (request._type == REQ_TYPE_CGI) {
		path = request._resHints.cgiRules->root + request._parsedUri.path;
		res = checkRessourcePath(path, REQ_TYPE_CGI);
	} else {
		if (request._method == POST) {
			path = (request._resHints.locationRules->upload_root != "" ? \
							request._resHints.locationRules->upload_root : \
							request._resHints.locationRules->root);
			res = checkRessourcePath(path + request._parsedUri.root, REQ_TYPE_DIR);
		} else {
			path = request._resHints.locationRules->root + request._parsedUri.path;
			res = checkRessourcePath(path, request._type);
		}
	}
	request._resHints.status = res;
	return (res);
}

/** @brief Check request validity from an host perspective.
 * @note get location/cgi match
	- check redirects
	- if dir && default_uri
		- change uri
		- check again for cgi
	- check location rules
	- check ressource path
 **/
int	Host::checkRequest(Request& request) const {
	int	res = 0;

	if ((res = matchRequest(request)))
		return (res);
	if (request._resHints.locationRules && (res = checkRedirection(request)))
		return (res);
	if (request._type == REQ_TYPE_DIR) { //If folder
		if ((res = checkDirRessource(request)))
			return (res);
	}
	if (request._resHints.locationRules && (res = checkLocationRules(request)))
		return (res);
	if (request._type == REQ_TYPE_CGI && (res = checkCGIRules(request)))
		return (res);
	if ((res = checkRessourceExistence(request)))
		return (res);
	return (res);
}
