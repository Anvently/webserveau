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

		virtual ~IControl(void) = 0;

		static int	handleListenEvent(epoll_event* event);
		static int	handleClientEvent(epoll_event* event);
		static int	handleCGIEvent(epoll_event* event);

		static int	handleClientIn(epoll_event* event);
		static int	handleClientOut(epoll_event* event);
		static int	handleClientHup(epoll_event* event);

		static int	parseCommandPrompt(std::deque<std::string>& words);

	public :

		static int	registerCommandPrompt(int epollfd);

		static int	handleEpoll(epoll_event* events, int nbrEvents);
		static int	handleCommandPrompt(epoll_event* event);

};

#endif
