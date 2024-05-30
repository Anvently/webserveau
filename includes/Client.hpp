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
# define BUFFER_SIZE 4096
#endif

#define HEADER_STATUS_ONGOING 0
#define HEADER_STATUS_READY 1
#define HEADER_STATUS_DONE 2

#define BODY_STATUS_NONE 0
#define BODY_STATUS_ONGOING 1
#define BODY_STATUS_DONE 2


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
		Request*			_request;
		AResponse*			_response;

		time_t				_lastInteraction;

		int					_headerStatus;
		int					_bodyStatus;
		int					_mode;
		std::string			_buffer;
		std::string			_fileName;
		std::ofstream*		_bodyStream;
		URI					_URI;

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
		const Host*			getHost() const;
		const std::string&	getStrAddr(void) const;
		int					getAddrPort(void) const;

		int					getRequestStatus() const; //returns 1 if request has been fully received
		int					getResponseStatus() const; // returns 1 if response is ready to send
		Request*			getRequest(); // returns the current not complete request or allocate a new one
		Request				*getFrontRequest(); // return the oldest request
		AResponse*			getResponse() const; // returns the current not complete response

		/// @brief
		/// @param buffer null terminated buffer
		/// @return ```< 0```
		int					parseRequest(const char* buffer);

		int					getHeaderStatus() const;
		int					getBodyStatus() const;
		int					getMode() const;
		const std::string&	getBodyFile() const;
		void				setMode(int mode);
		void				setHeaderStatus(int);
		void				setBodyStatus(int);
		void				setBodyMaxSize(int);
		// void				setBodyStream(std::ofstream*);
		void				setBodyFile(const std::string&);
		void				stashBuffer(std::string &buffer);
		void				retrieveBuffer(std::string &str);
		void				clearBuffer();
		void				updateLastInteraction(void);

		void				setHost(const std::string& hostname);
		void				setResponse(AResponse*);

		void				shutdownConnection(void);
		void				deleteFile();

};

std::ostream&	operator<<(std::ostream& os, const Client&);

#endif
