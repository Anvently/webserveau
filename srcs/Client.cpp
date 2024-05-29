#include <Client.hpp>
#include <netinet/ip.h>
#include <netinet/in.h>
#include <ctime>
#include <arpa/inet.h>
#include <ILogger.hpp>

std::list<Client>	Client::_clientList;

Client::~Client(void) {

	if (_request)
		delete(_request);
	if (_response)
		delete(_response);
	if (_filestream)
		_filestream->close();
}

Client::Client(const ClientSocket& socket, ListenServer& listenServer) \
	: _socket(socket), _host(NULL), _listenServer(listenServer), _headerStatus(HEADER_STATUS_ONGOING), \
	_bodyStatus(BODY_STATUS_NONE), _mode(READ)
{
	_port = 0;
	_lastInteraction = time(NULL);
	_request = NULL;
	_response = NULL;
	_filestream = NULL;
}

Client::Client(const Client& copy) : _socket(copy._socket), _addressStr(copy._addressStr), \
	_port(copy._port), _host(copy._host), _listenServer(copy._listenServer), \
	 _lastInteraction(copy._lastInteraction), \
	_headerStatus(copy._headerStatus), _bodyStatus(copy._bodyStatus), _mode(copy._mode), _buffer(copy._buffer), \
	_fileName(copy._fileName), _filestream(copy._filestream)
{
	this->_request = NULL;
	this->_response = NULL;
}

int	Client::getTotalNbrClient(void) {
	return (_clientList.size());
}

void	Client::clearBuffers(void) {
	while (!_outBuffers.empty()) {
		char* buffer = _outBuffers.front();
		delete buffer;
		_outBuffers.pop();
	}
}

std::list<Client>::iterator	Client::findClient(Client* client)
{
	std::list<Client>::iterator pos;
	for (pos = _clientList.begin(); pos != _clientList.end(); pos++)
	{
		if (client == &*pos)
			return (pos);
	}
	return (pos);
}

/// @brief Initialize a new client based on given socket.
/// Given socket must refer to a valid socket that was previously
/// returned via a call to ```accept()```.
/// @param socket
/// @return Pointer toward the client object that was created.
/// ```NULL``` should means an unexpected (and perhaps fatal) error
/// occured.
Client*	Client::newClient(const ClientSocket& socket, ListenServer& listenServer)
{
	char	strIp[64];
	Client	clientTmp(socket, listenServer);

	Client& client = *_clientList.insert(_clientList.end(), clientTmp);
	inet_ntop(AF_INET, &(((struct sockaddr_in*)&client._socket.addr)->sin_addr),
					strIp, 64);
	client._port = htons((((struct sockaddr_in *)&client._socket.addr.sa_data)->sin_port));
	client._addressStr += strIp;
	return (&client);
}

/// @brief Permanently delete a client.
/// @warning Will invalidate every reference to this client.
/// This operation should be done by the client's host
/// or the listenServer for orphan client and simultaneously with deleting
/// every reference to the client.
/// @param
void	Client::deleteClient(Client* client)
{
	if (client == NULL)
		return;
	client->shutdownConnection();
	std::list<Client>::iterator pos = findClient(client);
	if (pos != _clientList.end())
		_clientList.erase(pos);
}

int	Client::getfd(void) const {
	return (this->_socket.fd);
}

Host*	Client::getHost(void) const {
	return (this->_host);
}

const std::string&	Client::getStrAddr(void) const {
	return (this->_addressStr);
}

int	Client::getAddrPort(void) const {
	return (this->_port);
}

/// @brief Return the status of the request at the front of the queue.
/// @return ```-1``` if no request in the queue.
int	Client::getRequestStatus() const {
	if (this->_request != NULL)
		return (this->_request->getStatus());
	return (-1);
}

int	Client::getResponseStatus() const {
	// return (_response.getStatus());
	return (0);
}

void	Client::setHost(const std::string& hostname) {

	this->_host = this->_listenServer.bindClient(*this, hostname);
}

/// @brief
/// @param
void	Client::shutdownConnection(void) {
	close(_socket.fd);
}


Request*	Client::getRequest()
{
	if (_request == NULL)
		_request = new Request();
	return (_request);
}



void	Client::stashBuffer(std::string &str)
{
	this->_buffer += str;
}

void	Client::retrieveBuffer(std::string &str)
{
	str += this->_buffer;
	this->_buffer.clear();
}

void	Client::clearBuffer()
{
	this->_buffer.clear();
}

int	Client::parseRequest(const char* bufferIn) {
	std::string	fullBuffer = _buffer + bufferIn;
	Request*	request = getRequest();
	int			res = 0;

	LOGI("fullBuffer: %ss", &fullBuffer);
	if (_headerStatus < HEADER_STATUS_READY)
	{
		res = request->parseHeaders(fullBuffer);
		if (res < 0)
			_headerStatus = HEADER_STATUS_READY;
		return (res);
	}
	else if (_headerStatus == HEADER_STATUS_DONE && _bodyStatus == ONGOING)
	{
		res = request->parseInput(fullBuffer, _filestream);
		if (res < 0)
			_bodyStatus = BODY_STATUS_DONE;
		return(res);
	}
	else
		return (0);




	// while (!fullBuffer.empty() && request->getStatus() < 4)
	// {
	// 	res = request->parseInput(fullBuffer);
	// 	// if (res < 0 && (AssignHost(client) || req_ptr->getLenInfo()))
	// 	// {
	// 	// 	req_ptr->_fillError(400, "Host header missing or invalid");
	// 	// 	req_ptr->setStatus(COMPLETE);
	// 	// 	client->setStatus(ERROR);
	// 	// 	fullBuffer.clear();
	// 	// 	break ;
	// 	// }
	// 	if (res > 0)
	// 	{

	// 		fullBuffer.clear();
	// 		break ;
	// 	}
	// }
	// if (request->getStatus() == COMPLETE && _status != ERROR)
	// {
	// 	stashBuffer(fullBuffer);
	// 	req_ptr = client->getRequest();
	// 	req_ptr->printHeaders();
	// 	LOGE("Request status: %d | Client status: %d", client->getRequestStatus(), client->getStatus());
	// }
}

std::ostream&	operator<<(std::ostream& os, const Client& client) {
	os << "		->  fd: " << client._socket.fd << std::endl;
	os << "		ip/port: " << client._addressStr << ':' << client._port << std::endl;
	os << "		host: " << client._host;
	if (client._host)
		os << " (" << client._host->getServerNames().at(0) << ')';
	os << std::endl;
	if (client._request)
		os << " | status = " << client._request->getStatus() << " | error = " \
			<< client._request->getError();
	os << std::endl;
	os << "		last interaction: " << time(NULL) - client._lastInteraction << std::endl;
	os << "		nbr of out buffers: " << client._outBuffers.size() << std::endl;
	return (os);
}


int	Client::getHeaderStatus()
{
	return (this->_headerStatus);
}

int	Client::getBodyStatus()
{
	return (this->_bodyStatus);
}

int	Client::getMode()
{
	return (this->_mode);
}

void	Client::deleteFile()
{
	if (_filestream)
		_filestream->close();
	unlink(_fileName.c_str());
}
