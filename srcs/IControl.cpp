#include <IControl.hpp>


int	IControl::handleEpoll(struct epoll_event* events, int nbr_event)
{
	Client*	ptr_client;
	ListenServer*	ptr_listenS;

	if (nbr_event < 1)
		return (0);
	for (int i{0}; i < nbr_event; i++)
	{
		if ((ptr_listenS = dynamic_cast<ListenServer *> ((IObject *)events[i].data.ptr)) != NULL)
			handleListenEvent(&events[i]);
		else if ((ptr_client = dynamic_cast<Client *> ((IObject *) events[i].data.ptr)) != NULL && handleClientEvent(&events[i]))
			handleClientEvent(&events[i]);
		//handle CGI event
		//.....

	}
	// a round of parsing has been done, could try to assign clients to hosts
	//also need to check to timeout some connections
	return (0);
}


/// @brief invoked when the event is a brand new connection to a server, should create a new client.
/// @param event the event containing a pointer to the listenServer concerned
/// @return 0 on success 1 on some fatal error
int	IControl::handleListenEvent(epoll_event* event)
{
	ListenServer*	pt = static_cast<ListenServer *> (event->data.ptr);
	Client			*newClient = NULL;
	if ((newClient = pt->acceptConnection()) == NULL)
		return (1);
}


int	IControl::handleClientEvent(epoll_event *event)
{

}
