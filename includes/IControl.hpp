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

enum	RES_SENT_SITUATION {SITUATION_CLOSE = -1, SITUATION_KEEP_ALIVE = 1, SITUATION_CONTINUE = 2};

class IControl
{
	private :


		virtual 	~IControl(void) = 0;

		static int	handleListenEvent(epoll_event* event);
		static int	handleClientEvent(epoll_event* event, Client&);

		static int	handleClientIn(Client&, bool incoming = true);
		static int	handleClientOut(Client&);
		static int	handleClientHup(Client&);

		static int	handleClientResponse(Client&, int);

		static int	handleRequestHeaders(Client& client, Request& request);
		static int	defineBodyParsing(Client& client, Request& request);
		static int	handleRequestBodyDone(Request& request);
		static void	fillErrorPage(const Host* host, ResHints& request);
		static void	fillAdditionnalHeaders(Request& request);
		static void	fillVerboseError(Request& request);
		static void	fillResponse(Client& client, Request& request);

		static int	checkForbiddenHeaders(Request& request);
		static int	assignHost(Client& client, Request& request);
		static int	checkBodyLength(Client& client, Request& request);
		static int	checkContinue(Client& client, Request& request);

		static int	handleDeleteMethod(Request& request);
		static int	handleCGIProcess(Client& client);

		static void	handleKillCommand(std::deque<std::string>& words);
		static void	handlePrintCommand(std::deque<std::string>& words);

		static int	parseCommandPrompt(std::deque<std::string>& words);

	public :

		static int	_epollfd;

		static int	registerCommandPrompt(void);
		static int	registerToEpoll(int fd, void* data, int flags);
		static int	removeFromEpoll(int fd);

		static int	handleEpoll(epoll_event* events, int nbrEvents);
		static int	handleCommandPrompt(epoll_event* event);

		static void	generateContinueResponse(Client& client);
		static int	generateResponse(Client& client, int status = 0);
		static int	generateCGIProcess(Client& client);
		static int	cleanExit(int code);

};

#endif
