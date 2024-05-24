#include <Client.hpp>
#include <netinet/ip.h>
#include <netinet/in.h>
#include <arpa/inet.h>

std::list<Client>	Client::_clientList;

Client::~Client(void) {}

Client::Client(const ClientSocket& socket, ListenServer& listenServer) \
	: _socket(socket), _listenServer(listenServer)
{

}

void	Client::clearBuffers(void) {
	while (!_outBuffers.empty()) {
		char* buffer = _outBuffers.front();
		delete buffer;
		_outBuffers.pop();
	}
}

char *get_ip_str(const struct sockaddr *sa, char *s, size_t maxlen)
{
	switch(sa->sa_family) {
		case AF_INET:
			inet_ntop(AF_INET, &(((struct sockaddr_in *)sa)->sin_addr),
					s, maxlen);
			break;

		case AF_INET6:
			inet_ntop(AF_INET6, &(((struct sockaddr_in6 *)sa)->sin6_addr),
					s, maxlen);
			break;

		default:
			strncpy(s, "Unknown AF", maxlen);
			return NULL;
	}
	htons((((struct sockaddr_in *)clients[i].addr.sa_data)->sin_port)));
	return s;
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
	Client	client(socket, listenServer);

	_clientList.push_back(client);
	inet_ntop(AF_INET, &(((struct sockaddr_in*)&client._socket.addr)->sin_addr),
					strIp, 64);
	client._port = htons((((struct sockaddr_in *)&client._socket.addr.sa_data)->sin_port));
	return (&client);
}
