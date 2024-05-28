#include <IControl.hpp>
#include <string>
#include <sstream>
#include <IParseConfig.hpp>

int	IControl::_epollfd = -1;

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
			handleClientEvent(&events[i]);
		//handle CGI event
		//.....

	}
	// a round of parsing has been done, could try to assign clients to hosts
	//also need to check to timeout some connections
	return (0);
}

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

int	IControl::parseCommandPrompt(std::deque<std::string>& words) {
	if (words[0] == "stop" && words.size() > 1) {
		if (words[1] == "all") {
			LOGI("Stopping servers...");
			ListenServer::removeServers();
		}
		else if (words.size() > 2) {
			ListenServer::removeServer(words[1], words[2]);
		}
	}
	else if (words[0] == "total")
		LOGI("Nbr of clients = %d", Client::getTotalNbrClient());
	else
		LOGI("Invalid command");
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


int	IControl::handleClientEvent(epoll_event *event)
{
	if (event->events & EPOLLIN) {
		IControl::handleClientIn(event);
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

int	IControl::handleClientIn(epoll_event *event)
{
	Client	*client;
	char	buffer_c[BUFFER_SIZE + 1];
	int		n_read;
	int		res = 0;
	Request	*req_ptr;
	std::string	buffer;

	client = static_cast<Client *>(event->data.ptr);
	if (client->getStatus() != READ)
		return (0);
	if ((n_read = read(client->getfd(), buffer_c, BUFFER_SIZE)) < 0)
	{
		return (-1);
		//NEED TO REMOVE THIS CLIENT FATAL ERROR
	}
	buffer_c[n_read] = 0;
	client->retrieveBuffer(buffer);
	buffer += buffer_c;
	LOGI("buffer: %ss", &buffer);
	req_ptr = client->getRequest();
	while (!buffer.empty() && req_ptr->getStatus() == ONGOING)
	{
		res = req_ptr->parseInput(buffer);
		if (res < 0 && (AssignHost(client) || req_ptr->getLenInfo()))
		{
			req_ptr->_fillError(400, "Host header missing or invalid");
			req_ptr->setStatus(COMPLETE);
			client->setStatus(ERROR);
			buffer.clear();
			break ;
		}
		if (res > 0)
		{
			client->setStatus(ERROR);
			buffer.clear();
			break ;
		}

	}
	if (client->getRequestStatus() == COMPLETE && client->getStatus() != ERROR)
	{
		client->stashBuffer(buffer);
		client->setStatus(WRITE);
		req_ptr = client->getFrontRequest();
		req_ptr->printHeaders();
		LOGE("Request status: %d | Client status: %d", client->getRequestStatus(), client->getStatus());
	}
	return (0);
}
