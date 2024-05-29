#include <Client.hpp>
#include <netinet/ip.h>
#include <netinet/in.h>
#include <ctime>
#include <arpa/inet.h>
#include <ILogger.hpp>

std::list<Client>	Client::_clientList;

Client::~Client(void) {}

Client::Client(const ClientSocket& socket, ListenServer& listenServer) \
	: _socket(socket), _host(NULL), _listenServer(listenServer), _status(READ)
{
	_port = 0;
	_lastInteraction = time(NULL);
}

Client::Client(const Client& copy) : _socket(copy._socket), _addressStr(copy._addressStr), \
	_port(copy._port), _host(copy._host), _listenServer(copy._listenServer), \
	_requests(copy._requests), _response(copy._response), _lastInteraction(copy._lastInteraction), \
	_status(copy._status), _buffer(copy._buffer)
{
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
	if (this->_requests.empty() == false)
		return (_requests.front().getStatus());
	return (-1);
}

int	Client::getResponseStatus() const {
	// return (_response.getStatus());
	return (0);
}

void	Client::setHost(std::string hostname) {

	this->_host = this->_listenServer.bindClient(*this, hostname);
}

/// @brief
/// @param
void	Client::shutdownConnection(void) {
	close(_socket.fd);
}


Request&	Client::getRequest()
{
	if (this->_requests.empty() || this->_requests.back().getStatus() == COMPLETE)
	{
		Request	newRequest;
		this->_requests.push(newRequest);
	}
	return (this->_requests.back());
}

Request	*Client::getFrontRequest()
{
	if (this->_requests.empty())
		return (NULL);
	return (&this->_requests.front());
}

int	Client::getStatus()
{
	return (this->_status);
}

void	Client::setStatus(int st)
{
	this->_status = st;
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
	Request&	request = getRequest();
	int			res = 0;

	LOGI("fullBuffer: %ss", &fullBuffer);
	while (!fullBuffer.empty() && request.getStatus() == ONGOING)
	{
		res = request.parseInput(fullBuffer);
		if (res < 0)
			setStatus(HEADER_STATUS_READY);
		if (res > 0)
		{
			_status = ERROR;
			fullBuffer.clear();
			break ;
		}
	}
	if (request.getStatus() == COMPLETE && _status != ERROR)
	{
		stashBuffer(fullBuffer);
		req_ptr = client->getFrontRequest();
		req_ptr->printHeaders();
		LOGE("Request status: %d | Client status: %d", client->getRequestStatus(), client->getStatus());
	}
}

std::ostream&	operator<<(std::ostream& os, const Client& client) {
	os << "		->  fd: " << client._socket.fd << std::endl;
	os << "		ip/port: " << client._addressStr << ':' << client._port << std::endl;
	os << "		host: " << client._host;
	if (client._host)
		os << " (" << client._host->getServerNames().at(0) << ')';
	os << std::endl;
	os << "		current requests: " << client._requests.size();
	if (client._requests.size())
		os << " | status = " << client._requests.front().getStatus() << " | error = " \
			<< client._requests.front().getError();
	os << std::endl;
	os << "		last interaction: " << time(NULL) - client._lastInteraction << std::endl;
	os << "		nbr of out buffers: " << client._outBuffers.size() << std::endl;
	return (os);
}
