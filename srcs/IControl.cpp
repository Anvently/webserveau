#include <IControl.hpp>

int	IControl::handleListenEvent(epoll_event* event)
{
	(void) event;
	return (0);
}
int	IControl::handleClientEvent(epoll_event* event)
{
	(void) event;
	return (0);
}
int	IControl::handleCGIEvent(epoll_event* event)
{
	(void) event;
	return (0);
}

int	IControl::handleClientIn(epoll_event* event)
{
	(void) event;
	return (0);
}
int	IControl::handleClientOut(epoll_event* event)
{
	(void) event;
	return (0);
}
int	IControl::handleClientHup(epoll_event* event)
{
	(void) event;
	return (0);
}

int	IControl::handleEpoll(epoll_event* events, int nbrEvents)
{
	(void) events;
	(void) nbrEvents;
	return (0);
}
