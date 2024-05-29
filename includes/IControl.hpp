#ifndef ICONTROL_HPP
# define ICONTROL_HPP

#include <sys/epoll.h>
#include <IObject.hpp>
#include <Response.hpp>
#include <Host.hpp>
#include <Request.hpp>
#include <ListenServer.hpp>
#include <Client.hpp>

#ifndef BUFFER_SIZE
# define BUFFER_SIZE 4096
#endif

class IControl
{
	private :


		virtual 	~IControl(void) = 0;

		static int	handleListenEvent(epoll_event* event);
		static int	handleClientEvent(epoll_event* event);
		static int	handleCGIEvent(epoll_event* event);

		static int	handleClientIn(epoll_event* event);
		static int	handleClientOut(epoll_event* event);
		static int	handleClientHup(epoll_event* event);

		/**
		@brief Should be called once the full header is parsed
			- if header READY
				- check forbidden headers; accept-ranges, content-encoding, transfrer-encoding != chuked
				- check host rules
					- identify host
					- check headers content-length
				- parse uri, get location (and parameters) and check for body
					- check redirects
					- if dir && default_uri
						- change uri
					- check if cgi or static/dir_listing
					- check allowed methods
					- if not static POST
						- check ressource existence
					- else
						- check upload settings
					- if continue
						- generate body parsing config (creating ostream and max_chunk_length)
						- genereate CONTINUE RESPONSE
					- else if body
						- resume body parsing as CGI
			- WHILE (CONTINUE RESPONSE NOT SENT) && !NOT ERROR
				- return
				- if body
					- resume body parsing with given config
						- switch to READ
			- if BODY READY && !ERROR
				- generate response
					- CGI
					- dir_listing
					- STATIC page
					- switch to WRITE
			- while (response NOT READY)
				- wait
			- send response
		**/
		static int	handleClientRequest(Client& client);

		/**
		@brief check forbidden headers; accept-ranges, content-encoding, transfrer-encoding != chuked
		**/
		static int checkForbiddenHeaders(void);
		/**
		@brief check if host is given, empty or not, assign correct or first host found.
		Check content-length
		**/
		static int checkHost(void);

		static void	handleKillCommand(std::deque<std::string>& words);
		static void	handlePrintCommand(std::deque<std::string>& words);

		static int	parseCommandPrompt(std::deque<std::string>& words);

	public :

		static int	_epollfd;

		static int	registerCommandPrompt(void);
		static int	registerToEpoll(int fd, void* data, int flags);

		static int	handleEpoll(epoll_event* events, int nbrEvents);
		static int	handleCommandPrompt(epoll_event* event);

};

#endif
 