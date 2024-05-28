#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <IObject.hpp>
#include <Response.hpp>
#include <Host.hpp>
#include <Request.hpp>
#include <ListenServer.hpp>
#include <ctime>
#include <queue>
#include <sys/socket.h>
#include <stdint.h>


// class Request;
// class Host;
// class ListenServer;
// class Response;

#define MAX_NBR_OUT_BUFFERS 10

#ifndef BUFFER_SIZE
# define BUFFER_SIZE 4092
#endif


enum {READ, WRITE, ERROR};

struct ClientSocket
{
	int				fd;
	struct sockaddr	addr;
	socklen_t		addrSize;
};

class	Client : public IObject
{

	private:

		static std::list<Client>	_clientList;

		Client();
		Client(const ClientSocket& socket, ListenServer& listenServer);
		// Client(int fd, Host *host, Request *req, Response *resp);

		ClientSocket		_socket;

		std::string			_addressStr;
		int					_port;

		Host*				_host;
		ListenServer&		_listenServer;
		std::queue<Request>	_requests;
		Response			_response;

		time_t				_lastInteraction;

		int					_status;
		std::string			_buffer;

		// May want something more versatile
		// (If CGI, it would be linked to a pipe
		// If static, it would be linked to an ifstream)
		std::queue<char*>	_outBuffers;

		void	clearBuffers(void);

		static std::list<Client>::iterator	findClient(Client* client);

		friend Client* 			ListenServer::acceptConnection(void);
		friend std::ostream&	operator<<(std::ostream& os, const Client&);


	public:

		Client(const Client &Copy);
		virtual ~Client();

		static Client*		newClient(const ClientSocket& socket, ListenServer& listenServer);
		static void			deleteClient(Client* client);

		static int			getTotalNbrClient(void);

		int					getfd() const;
		Host*				getHost() const;
		const std::string&	getStrAddr(void) const;
		int					getAddrPort(void) const;

		int					getRequestStatus() const; //returns 1 if request has been fully received
		int					getResponseStatus() const; // returns 1 if response is ready to send
		Request				*getRequest(); // returns the current not complete request
		Request				*getFrontRequest(); // return the oldest request
		Response			*getResponse(); // returns the current not complete response

		int					getStatus();
		void				setStatus(int st);
		void				stashBuffer(std::string &buffer);
		void				retrieveBuffer(std::string &str);
		void				clearBuffer();

		void				setHost(std::string hostname);

		void				shutdownConnection(void);

};

std::ostream&	operator<<(std::ostream& os, const Client&);

#endif
