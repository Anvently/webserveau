#include <IControl.hpp>
#include <string>
#include <sstream>
#include <IParseConfig.hpp>
#include <Response.hpp>

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
	int	res = 0;

	if ((event->events & EPOLLIN) && client.getMode() == READ) {
		if (res = IControl::handleClientIn(client)) {
			generateResponse(client, res);
			client.setMode(WRITE);
		}
	}
	else if ((event->events & EPOLLOUT) && client.getMode() == WRITE)
		return (0);
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

/**
@brief check forbidden headers; accept-ranges, content-encoding, transfrer-encoding != chuked
**/
int	IControl::checkForbiddenHeaders(const Request& request) {
	if (1)
		return (RES_NOT_IMPLEMENTED);
	return (0);
}

/**
@brief check if host is given, empty or not, assign correct or first host found.
Check content-length
**/
int	IControl::assignHost(Client& client, const Request& request) {
	if (request.checkHeader("host") == false)
		return (RES_BAD_REQUEST);
	client.setHost(request.getHeader("host"));
	return (0);
}

/// @brief Check if header tells about the existence of a body and if it 
/// match host mast body size, if a body is present, update client body status
/// to ```ONGOING```.
/// @param client 
/// @param request 
/// @return ```status``` of the error or ```0``` if no error.
int	IControl::checkBodyLength(Client& client, const Request& request) 
{
	int bodyLength = -1;
	if (request.checkHeader("content-length") == true) {
		getInt(request.getHeader("content-length"), 10, bodyLength);
		if (bodyLength > client.getHost()->getMaxSize())
			return (RES_REQUEST_ENTITY_TOO_LARGE);
	}
	if (bodyLength != -1 || request.getHeader("transfer-encoding") == "chunked")
	{
		client.setBodyStatus(BODY_STATUS_ONGOING);
		client.setBodyMaxSize(client.getHost()->getMaxSize());
	}
	return (0);
}

/**
	@brief Should be called once the full header is parsed
		- if header READY
			- check forbidden headers; accept-ranges, content-encoding, transfrer-encoding != chuked
			- check host and assign one to client
			- check if there is a body provided
			- parse uri
			- check with host if the request must be rejected
				- if continue
					- generate body parsing config (creating ostream and max_chunk_length)
					- genereate CONTINUE RESPONSE
				- else if body
					- resume body parsing as CGI
	@return ```status``` of the response, or ```100``` for continue response

**/
int	IControl::handleRequestHeaders(Client& client, Request& request) {
	int	res = 0, type = 0;

	client.setBodyStatus(BODY_STATUS_NONE);
	if ((res = request.parseURI()))
		return (res);
	if ((res = checkForbiddenHeaders(request)))
		return (res);
	if ((res = assignHost(client, request)))
		return (res);
	if ((res = checkBodyLength(client, request)))
		return (res);
	if ((res = client.getHost()->checkRequest(request)))
		return (res);
	if (request.getHeader("expect") == "100-continue")
		res = RES_CONTINUE;
	else if (request.getHeader("expect") != "")
		return (RES_EXPECTATION_FAILED);
	client.setHeaderStatus(HEADER_STATUS_DONE);
	return (res);
}

int	IControl::defineBodyParsing(Client& client, const Request& request)
{
	if (request.getType() == REQ_TYPE_CGI)
		client.setBodyFile(generate_name(client.getHost()->getServerNames().front()));
	else if (request.getType() == REQ_TYPE_STATIC && request.getMethod() == POST)
		client.setBodyFile(request.getCGIConfig()->root + request.getUri().path);
	else
		client.setBodyFile("");
}

int	IControl::handleRequestBodyDone(Request& request)
{
	if (request.getType() == REQ_TYPE_STATIC) {
		if (request.getMethod() == POST)
			return (RES_CREATED);
		else if (request.getMethod() == DELETE)
			return (RES_NO_CONTENT);
	}
	return (RES_OK);
}

int	IControl::handleClientIn(Client& client)
{
	char	buffer_c[BUFFER_SIZE + 1];
	int		n_read, res = 0;

	if (client.getMode() != READ)
		return (0);
	if ((n_read = read(client.getfd(), buffer_c, BUFFER_SIZE)) < 0)
		return (-1); //NEED TO REMOVE THIS CLIENT FATAL ERROR
	
	buffer_c[n_read] = 0;
	res = client.parseRequest(buffer_c);
	if (res < 0) { //Status changed
		if (client.getHeaderStatus() == HEADER_STATUS_READY) {
			res = handleRequestHeaders(client, *client.getRequest());
			defineBodyParsing(client, *client.getRequest());
		}
		if (res == 0 && client.getBodyStatus() != BODY_STATUS_ONGOING)
			res = handleRequestBodyDone(*client.getRequest());
	}
	return (res);
}

/// @brief Generate an appropriate response type from the given
/// status.
/// @param client 
/// @param status ```0``` if the response to generate is not an error
/// @note Possible hints for response
///				- headers
///				- final path
///				- 
void	IControl::generateResponse(Client& client, int status)
{
	if (client.getResponse()) //Not sure
		return ; //Not sure

	switch (status)
	{
		case RES_CONTINUE:
			client.setResponse(new SingleLineResponse(100, "Continue"));
			break;
		
		case RES_OK: //For Static/dir GET or CGI operation
			/*
				If cgi
					- file path
					- cgiConfig
					- method && headers
				If dir_listing
					- locationRule
				If static
					- file path
			*/
			break;

		case RES_CREATED: //Need path
			// client.setResponse(new StaticPageResponse());
			break;

		case RES_NO_CONTENT: //ok but no body, for DELETE
			
			break;

		case RES_MULTIPLE_CHOICE:
			//List of redirections in body, with the first one in location header
			//Dynamic body
			break;

		case RES_MOVED_PERMANENTLY:
			//The new URI should be given in location field
			//Dynamic body containing html redirection
			break;

		case RES_FOUND:
			//The new URI should be given in location field
			//Dynamic body containing html redirection
			break;
		
		case RES_SEE_OTHER:
			//The new URI should be given in location field
			//Dynamic body containing html redirection
			break;

		case RES_TEMPORARY_REDIRECT:
			//The new URI should be given in location field
			//Dynamic body containing html redirection
			break;

		case RES_BAD_REQUEST: //Verbose hint
			//Dynamic body
			break;

		case RES_NOT_FOUND: //Full static page
		
		break;

		case RES_METHOD_NOT_ALLOWED: //Full static page, must contains allow header
		
		break;

		case RES_TIMEOUT: //Full static page
		
		break;

		case RES_LENGTH_REQUIRED: //Full static page
		
		break;

		case RES_REQUEST_ENTITY_TOO_LARGE: //Full static page
		
		break;

		case RES_REQUEST_URI_TOO_LONG: //Full static page
		
		break;

		case RES_EXPECTATION_FAILED: //Full static page
		
		break;

		case RES_INTERNAL_ERROR: //Full static page
		
		break;

		case RES_NOT_IMPLEMENTED: //Full static page
		
		break;

		default:
			LOGE("Unknow status: %d", status);
			break;
	}
}
