#include <IControl.hpp>
#include <string>
#include <sstream>
#include <IParseConfig.hpp>

int	IControl::_epollfd = -1;

int	IControl::registerCommandPrompt(void) {
	struct epoll_event	event;

	event.events = EPOLLIN | EPOLLHUP;
	event.data.fd = STDIN_FILENO;
	if (epoll_ctl(_epollfd, EPOLL_CTL_ADD, STDIN_FILENO, &event))
		return (-1);
	return (0);
}

int	IControl::registerToEpoll(int fd, void* ptr, int flags) {
	struct epoll_event	event;

	event.events = flags;
	event.data.ptr = ptr;
	if (epoll_ctl(_epollfd, EPOLL_CTL_ADD, fd, &event))
		return (-1);
	return (0);
}

int	IControl::handleCommandPrompt(epoll_event* event) {
	if (event->events & EPOLLHUP || std::cin.eof()) {
		LOGI("Closing stdin");
		ListenServer::removeServers();
		return (1);
	}
	else if ((event->events & EPOLLIN) == 0)
		return (0);
	std::string				input;
	std::deque<std::string>	words;

	std::getline(std::cin, input);
	std::stringstream		inputStream(input);
	IParseConfig::parseValues(inputStream, words, 100);
	return (parseCommandPrompt(words));
}

void	IControl::handleKillCommand(std::deque<std::string>& words)
{
	if (words.size() < 2)
		LOGV("Invalid number of argument.");
	else if (words[1] == "all")
		ListenServer::removeServers();
	else if (words.size() == 3) {
			LOGV("Trying to remove server %ss:%ss", &words[1], &words[2]);
			ListenServer::removeServer(words[1], words[2]);
	}
	else if (words.size() == 4) {
		if (ListenServer::serverExist(words[1], words[2]))
		{
			std::list<ListenServer>::iterator it = ListenServer::findServer(words[1], words[2]);
			if (it->removeHost(words[3]))
				LOGV("Host (%ss) was not found in server %ss:%ss", &words[3], &words[1], &words[2]);
		}
		else
			LOGV("Invalid server");
	}
	else
			LOGV("Invalid number of argument.");
}

void	IControl::handlePrintCommand(std::deque<std::string>& words)
{
	if (words.size() < 2) {
		for (std::list<ListenServer>::const_iterator it = ListenServer::getServerListBegin();
			it != ListenServer::getServerListEnd(); it++)
				LOGV("%Ls", &*it);
	}
	else if (words.size() > 2) {
		if (ListenServer::serverExist(words[1], words[2]))
		{
			std::list<ListenServer>::iterator it = ListenServer::findServer(words[1], words[2]);
			if (words.size() == 3)
				LOGV("%Ls", &*it);
			else if (words.size() >= 4) {
				Host*	host = it->findHost(words[3]);
				if (host)
					LOGV("%H", host);
				else
					LOGV("Invalid host");
			}
		}
		else
			LOGV("Invalid server");
	}
}

int	IControl::parseCommandPrompt(std::deque<std::string>& words) {
	if (words.size() == 0)
		return (0);
	else if (words[0] == "kill")
		handleKillCommand(words);
	else if (words[0] == "print")
		handlePrintCommand(words);
	else if (words[0] == "client_nbr")
		LOGV("Nbr of clients = %d", Client::getTotalNbrClient());
	else
		LOGV("Invalid command");
	return (0);
}


int	IControl::handleEpoll(struct epoll_event* events, int nbr_event)
{
	Client*	ptr_client;
	ListenServer*	ptr_listenS;

	if (nbr_event < 1)
		return (0);
	for (int i = 0; i < nbr_event; i++)
	{
		// LOGD("event => fd = %d | event = %d", events[i].data.fd, events[i].events);
		if (events[i].data.fd == STDIN_FILENO) {
			if (handleCommandPrompt(&events[i]))
				return (1);
		}
		else if ((ptr_listenS = dynamic_cast<ListenServer *> ((IObject *)events[i].data.ptr)) != NULL)
			handleListenEvent(&events[i]);
		else if ((ptr_client = dynamic_cast<Client *> ((IObject *) events[i].data.ptr)) != NULL)
			handleClientEvent(&events[i], *ptr_client);
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
	if (registerToEpoll(newClient->getfd(), newClient, EPOLLIN | EPOLLHUP))
		return (1);
	return (0);
}


int	IControl::handleClientEvent(epoll_event *event, Client& client)
{
	if (event->events & EPOLLIN) {
		IControl::handleClientIn(client);
		return (0);
	}
	else
		return (0);
}


int	AssignHost(Client *client)
{
	std::string	hostname;
	if (client->getFrontRequest()->getHostName(hostname))
		return (1);
	client->setHost(hostname);
	client->getFrontRequest()->setBodyMaxSize(client->getHost()->getMaxSize());
	return(0);
}

#define HEADER_STATUS_ONGOING 0
#define HEADER_STATUS_READY 1
#define HEADER_STATUS_DONE 2

#define BODY_STATUS_NONE 0
#define BODY_STATUS_ONGOING 1
#define BODY_STATUS_DONE 2

int	IControl::handleClientIn(Client& client)
{
	char	buffer_c[BUFFER_SIZE + 1];
	int		n_read, ret = 0, response = 0;

	if (client.getStatus() != READ)
		return (0);
	if ((n_read = read(client.getfd(), buffer_c, BUFFER_SIZE)) < 0)
		return (-1); //NEED TO REMOVE THIS CLIENT FATAL ERROR
	buffer_c[n_read] = 0;
	ret = client.parseRequest(buffer_c);
	if (ret > 0) //Error occured
		generateResponse(client, ret);
	else if (ret < 0) { //Status changed
		if (client.getHeaderStatus() == HEADER_STATUS_READY) {
			if ((ret = handleClientRequest(client)))
				generateResponse(client, ret);
		}
		if (client.getBodyStatus() == BODY_STATUS_DONE && ret == 0) {
			generateResponse(client);
		}
	}
	else //Nothing changed
		return (0);
	client.setMode(WRITE);
	return (0);
}

void	IControl::generateResponse(Client& client, int ret)
{
	if (response) {
		if (client.getResponse()) //Not sure
			delete (client.getResponse()); //Not sure
		client.setResponse(new Response(ret));
		
	}
}
