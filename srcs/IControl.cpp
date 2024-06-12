#include <IControl.hpp>
#include <string>
#include <sstream>
#include <IParseConfig.hpp>
#include <Response.hpp>
#include <CGIProcess.hpp>

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

int	IControl::removeFromEpoll(int fd) {
	if (epoll_ctl(_epollfd, EPOLL_CTL_DEL, fd, NULL))
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
	if (words.size() < 2 || words.size() > 4)
		LOGV("Invalid number of argument.\n"\
			"	- kill all\n"\
			"	- kill [server_ip] [port]\n"\
			"	- kill [server_ip] [port] [host]");
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
	else if (words[0] == "clear")
		system("clear");
	else
		LOGV("Invalid command");
	return (0);
}


int	IControl::handleEpoll(struct epoll_event* events, int nbr_event)
{
	Client*	ptr_client;
	ListenServer*	ptr_listenS;

	if (nbr_event < 1)
		return (-1);
	for (int i = 0; i < nbr_event; i++)
	{
		// LOGD("event => fd = %d | event = %d", events[i].data.fd, events[i].events);
		if (events[i].data.fd == STDIN_FILENO) {
			if (handleCommandPrompt(&events[i]))
				return (1);
		}
		else if ((ptr_listenS = dynamic_cast<ListenServer *> ((IObject *)events[i].data.ptr)) != NULL)
			handleListenEvent(&events[i]);
		else if ((ptr_client = dynamic_cast<Client *> ((IObject *) events[i].data.ptr)) != NULL) {
			if (handleClientEvent(&events[i], *ptr_client) < 0)
				ptr_client->terminate();
		}
		//handle CGI event
		//.....

	}
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
	if (registerToEpoll(newClient->getfd(), newClient, EPOLLIN | EPOLLHUP | EPOLLOUT))
		return (1);
	return (0);
}


/// @brief 
/// @param event 
/// @param client 
/// @return ```< 0``` if error or connection closed from client side
int	IControl::handleClientEvent(epoll_event *event, Client& client)
{
	int	res = 0;

	if ((event->events & EPOLLHUP) || client.getMode() == CLIENT_MODE_ERROR)
		handleClientHup(client);
	else if ((event->events & EPOLLIN) && ((client.getMode() == CLIENT_MODE_READ)
										|| (client.getBodyStatus() != BODY_STATUS_ONGOING))) {
		if ((res = IControl::handleClientIn(client))) {
			if (res > 0) {
				if (client.getRequest()->type != REQ_TYPE_CGI || res != RES_OK)
					res = generateResponse(client, res);
				else if ((res = generateCGIProcess(client)))
					res = generateResponse(client, RES_INTERNAL_ERROR);
			}
			if (res < 0)
				return (-1);
		}
	}
	else if ((event->events & EPOLLOUT) && client.getMode() == CLIENT_MODE_WRITE) {
		// LOGE("client fd = %d", client.getfd());
		if (client.cgiProcess && handleCGIProcess(client))
			return (-1);
		if (client.getResponse()) {
			if ((res = handleClientOut(client))) {
				if (res == SITUATION_KEEP_ALIVE) {
					client.clear();
					client.setMode(CLIENT_MODE_READ);
					return (0);
				} else if (res == SITUATION_CONTINUE) {
					client.clearResponse();
					client.setMode(CLIENT_MODE_READ);
				}
				else
					client.terminate();
			}
		}
	}
	return (0);
}

/// @brief 
/// @param client 
/// @return ``` < 0``` if error. ```> 0 (status)``` if a response can be generated 
int	IControl::handleClientIn(Client& client)
{
	char	buffer_c[BUFFER_SIZE + 1];
	int		n_read, res = 0;

	// if (client.getMode() != CLIENT_MODE_READ)
	// 	return (0);
	if ((n_read = read(client.getfd(), buffer_c, BUFFER_SIZE)) < 0)
		return (-1); //NEED TO REMOVE THIS CLIENT FATAL CLIENT_MODE_ERROR
	if (n_read == 0) //Connection closed
		return (-1);
	res = client.parseRequest(buffer_c, n_read);
	if (res < 0) { //Status changed
		if (client.getHeaderStatus() == HEADER_STATUS_READY) {
			if ((res = handleRequestHeaders(client, *client.getRequest())))
				return (res);
			if (client.getBodyStatus() != BODY_STATUS_NONE)
				res = client.parseRequest("", 0);
		}
		if (res <= 0 && client.getBodyStatus() != BODY_STATUS_ONGOING)
			res = handleRequestBodyDone(*client.getRequest());
	}
	return (res);
}

/// @brief
/// @param client
/// @return ```0``` if response if not sent yet, ```> 0``` if response sent
/// and connection is to be kept alive, ```< 0``` if connection is to be closed.
int	IControl::handleClientOut(Client& client) {
	int	nwrite = 0;

	if (client.outBuffers.size() >= 1) {
		nwrite = write(client.getfd(), client.outBuffers.front().c_str(), client.outBuffers.front().size());
		if (nwrite < 0) {
			LOGE("Write error");
			return (SITUATION_CLOSE);
		}
		client.outBuffers.pop();
	}
	if (client.outBuffers.empty()) {
		if (client.getRequest()->getHeader("expect") == "100-continue")
			return (SITUATION_CONTINUE);
		else if (client.getRequest()->resHints.headers["connection"] == "close")
			return (SITUATION_CLOSE);
		else
			return (SITUATION_KEEP_ALIVE);
	}
	return (0);
}

int	IControl::handleClientHup(Client& client) {
	client.terminate();
	LOGE("EPOLLHUP !!!");
	return (0);
}

/// @brief
/// @param client
/// @return
int	IControl::handleCGIProcess(Client& client) {
	int	res = client.cgiProcess->checkEnd();
	if (res == 0)
		return (0);
	if (res < 0)
		generateResponse(client, RES_SERVICE_UNAVAILABLE);
	else if (res > 0) {
		res = client.cgiProcess->parseHeaders();
		if (res == CGI_RES_DOC || res == CGI_RES_CLIENT_REDIRECT)
			generateResponse(client);
		else if (res == CGI_RES_LOCAL_REDIRECT) {
			handleRequestHeaders(client, *client.getRequest());
			generateResponse(client);
		}
	}
	if (client.cgiProcess)
		client.deleteCGIProcess();
	return (0);
}

// int	AssignHost(Client *client)
// {
// 	std::string	hostname;
// 	if (client->getRequest()->getHostName(hostname))
// 		return (1);
// 	client->setHost(hostname);
// 	client->getFrontRequest()->setBodyMaxSize(client->getHost()->getMaxSize());
// 	return(0);
// }

/**
@brief check forbidden headers; accept-ranges, content-encoding, transfrer-encoding != chuked
**/
int	IControl::checkForbiddenHeaders(Request& request) {
	if (request.getHeader("accept-ranges") != "") {
		request.resHints.verboseError = "accept-range header not implemented";
	} else if (request.checkHeader("content-encoding") && request.getHeader("content-encoding") != "identity") {
		request.resHints.status = RES_UNSUPPORTED_MEDIA_TYPE;
		return (RES_UNSUPPORTED_MEDIA_TYPE);
	} else if (request.checkHeader("transfer-encoding") && request.getHeader("transfer-encoding") != "chunked") {
		request.resHints.verboseError = "chunked is the only value supported for transfer-encoding";
	}
	else
		return (0);
	request.resHints.status = RES_NOT_IMPLEMENTED;
	return (RES_NOT_IMPLEMENTED);
}

/*
[4] line = GET / HTTP/1.1
[4] line = Host: localhost:8080
[3] line = GET / HTTP/1.1
[3] line = Host: localhost:8080


*/

/**
@brief check if host is given, empty or not, assign correct or first host found.
Check content-length
**/
int	IControl::assignHost(Client& client, Request& request) {
	if (request.checkHeader("host") == false) {
		request.resHints.verboseError = "request must include an host header";
		request.resHints.status = RES_BAD_REQUEST;
		return (RES_BAD_REQUEST);
	}
	client.setHost(request.getHeader("host"));
	return (0);
}

/// @brief Check if header tells about the existence of a body and if it
/// match host mast body size, if a body is present, update client body status
/// to ```ONGOING```.
/// @param client
/// @param request
/// @return ```status``` of the error or ```0``` if no error.
int	IControl::checkBodyLength(Client& client, Request& request)
{
	int bodyLength = -1;
	if (request.checkHeader("content-length") == true) {
		getInt(request.getHeader("content-length"), 10, bodyLength);
		if (bodyLength > client.getHost()->getMaxSize()) {
			request.resHints.status = RES_REQUEST_ENTITY_TOO_LARGE;
			return (RES_REQUEST_ENTITY_TOO_LARGE);
		}
		else if (bodyLength > 0)
			request.setContentLength(bodyLength);
	}
	if (bodyLength > 0 || request.getHeader("transfer-encoding") == "chunked")
	{
		if (request.getHeader("transfer-encoding") == "chunked")
			request.setChunked(true);
		client.setBodyStatus(BODY_STATUS_ONGOING);
		request.setBodyMaxSize(client.getHost()->getMaxSize());
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

localhost:80/dir1/subdir3/subdir.dir/index.php/file
parseuri->getpath
/dir1/subdir3/subdir.dir/index.php/file


extractPathInfo(URI&, extension) {

}

**/
int	IControl::handleRequestHeaders(Client& client, Request& request) {
	int	res = 0;

	client.setBodyStatus(BODY_STATUS_NONE);
	if ((res = assignHost(client, request)))
		return (res);
	if ((res = request.parseURI()))
		return (res);
	if ((res = checkForbiddenHeaders(request)))
		return (res);
	if ((res = checkBodyLength(client, request)))
		return (res);
	if ((res = client.getHost()->checkRequest(request)))
		return (res);
	if (request.getHeader("expect") == "100-continue")
		res = RES_CONTINUE;
	else if (request.getHeader("expect") != "") {
		request.resHints.status = RES_EXPECTATION_FAILED;
		return (RES_EXPECTATION_FAILED);
	}
	defineBodyParsing(client, *client.getRequest());
	client.setHeaderStatus(HEADER_STATUS_DONE);
	if (res == RES_CONTINUE) {
		generateContinueResponse(client);
		return (0);
	}
	return (res);
}

enum	fileSituation {FILE_DONT_EXIT, FILE_EXIST, FILE_IS_NOT_REG};

static int	checkFileExist(const char *path) {
	struct stat	f_stat;
	if (stat(path, &f_stat))
		return (FILE_DONT_EXIT);
	if (S_ISREG(f_stat.st_mode) == false)
		return (FILE_IS_NOT_REG);
	return (FILE_EXIST);
}

int	IControl::defineBodyParsing(Client& client, Request& request)
{
	if (request.type == REQ_TYPE_CGI) {
		if (client.getBodyStatus() != BODY_STATUS_NONE)
			client.setBodyFile(generate_name(&client.getHost()->getServerNames().front()));
		else
			client.setBodyFile("");
		request.resHints.unlink = true;
	}
	else if (request.type == REQ_TYPE_STATIC && request.method == POST) {
		std::string	filePath;
		filePath =  (request.resHints.locationRules->upload_root != "" ? \
					request.resHints.locationRules->upload_root : \
					request.resHints.locationRules->root) \
					+ request.resHints.parsedUri.path;
		switch (checkFileExist(filePath.c_str()))
		{
			case FILE_DONT_EXIT:
				request.resHints.alreadyExist = false;
				request.resHints.status = RES_CREATED;
				break;

			case FILE_EXIST:
				if (access(filePath.c_str(), W_OK) < 0) {
					request.resHints.status = RES_FORBIDDEN;
					return (RES_FORBIDDEN);
				}
				request.resHints.alreadyExist = true;
				request.resHints.status = RES_OK;
				break;

			case FILE_IS_NOT_REG:
				request.resHints.status = RES_FORBIDDEN;
				return (RES_FORBIDDEN);
		}
		request.resHints.path = filePath;
		client.setBodyFile(filePath);
		request.resHints.unlink = false;
	}
	else {
		client.setBodyFile("");
	}
	if (request.checkHeader("content-type"))
		request.resHints.headers["content-type"] = request.getHeader("content-type");
	return (0);
}

int	IControl::handleDeleteMethod(Request& request) {
	unlink(request.resHints.path.c_str());
	return (0);
}

int	IControl::handleRequestBodyDone(Request& request)
{
	if (request.type == REQ_TYPE_STATIC) {
		if (request.method == POST) {
			return (request.resHints.status);
		}
		else if (request.method == DELETE) {
			handleDeleteMethod(request);
			request.resHints.path = "";
			return (RES_NO_CONTENT);
		}
	}
	return (RES_OK);
}

std::string	IntToString(int x, int base);

/// @brief Check if there is an error status that requires to find the
/// corresponding static error page
/// @param host
/// @param request
void	IControl::fillErrorPage(const Host* host, ResHints& resHints) {
	if ((resHints.status > 400 && resHints.status < 418)
		|| resHints.status == 505)  {
		if (host)
			resHints.path = host->getDirErrorPages() \
				+ IntToString(resHints.status, 10) + ".html";
		else
			resHints.path =  IParseConfig::default_error_pages \
				+ IntToString(resHints.status, 10) + ".html";
	}
}

/// @brief Add any additionnal header such as ```connection``` if success status
/// @param request
void	IControl::fillAdditionnalHeaders(Request& request) {
	if ((request.resHints.status >= 200 && request.resHints.status < 300)
		|| request.resHints.status == 404) {
		if (request.checkHeader("connection"))
			request.resHints.headers["connection"] = request.getHeader("connection");
		else
			request.resHints.headers["connection"] = "keep-alive";
	}
	else
		request.resHints.headers["connection"] = "close";
}

void	IControl::fillVerboseError(Request& request) {
	std::string	allows;
	int			locMethod, cgiMethod;

	switch (request.resHints.status) {
		case RES_METHOD_NOT_ALLOWED:
			locMethod = (request.resHints.locationRules ? \
				request.resHints.locationRules->methods : INT32_MAX);
			cgiMethod = (request.resHints.cgiRules ? \
				request.resHints.cgiRules->methods : INT32_MAX);
			for (int i = 0; i < METHOD_NBR; i++) {
				if ((locMethod & (1 << i)) && (cgiMethod & (1 << i)))
					allows += (allows.size() ? "," : "") + METHOD_STR[i];
			}
			request.resHints.headers["Allow"] = allows;
			break;

		case RES_INTERNAL_ERROR:
			request.resHints.path = "";
			request.resHints.verboseError = "Internal error";
			break;

		case RES_SERVICE_UNAVAILABLE:
			request.resHints.path = "";
			request.resHints.verboseError = "Service temporarly unavailable.";
			break;

		case RES_BAD_REQUEST:
			request.resHints.path = "";
			break;

		default:
			break;
	}
}

void	IControl::fillResponse(Client& client, Request& request) {
	fillErrorPage(client.getHost(), request.resHints);
	fillAdditionnalHeaders(request);
	fillVerboseError(request);
	if (request.resHints.status)
		LOGI("Client %d : Generating response status %d | verbose = %ss",
			client.getfd(), request.resHints.status, &request.resHints.verboseError);
	request.resHints.type = request.type;
	request.resHints.extension = request.resHints.parsedUri.extension;
}

void	IControl::generateContinueResponse(Client& client) {
	client.setResponse(new SingleLineResponse(100, "100-continue"));
	client.getResponse()->writeResponse(client.outBuffers);
	// LOGD("continue generated");
	client.setMode(CLIENT_MODE_WRITE); //temporary
}

/// @brief Generate an appropriate response type from the given
/// status.
/// @param client
/// @param status
/// @return ```-1``` if error
int	IControl::generateResponse(Client& client, int status)
{
	AResponse*	response = NULL;
	Request&	request = *client.getRequest();
	if (client.getResponse()) //Not sure
		return (0); //Not sure
	if (status)
		request.resHints.status = status;
	fillResponse(client, request);
	try
	{
		response = AResponse::genResponse(request.resHints);
	}
	catch(const std::exception& e)
	{
		LOGE("Response exception : %s", e.what());
		if (request.resHints.status == RES_INTERNAL_ERROR)
			client.terminate();
		else {
			client.clearResponse();
			generateResponse(client, RES_INTERNAL_ERROR);
		}
		return (-1);
	}
	if (response)
		response->writeResponse(client.outBuffers);
	client.setResponse(response);
	client.setMode(CLIENT_MODE_WRITE);
	return (0);
}

int IControl::generateCGIProcess(Client& client) {
	client.cgiProcess = new CGIProcess(client);
	if (client.cgiProcess->execCGI())
		return (RES_INTERNAL_ERROR);
	client.setMode(CLIENT_MODE_WRITE);
	return (0);
}

int	IControl::cleanExit(int code) {
	if (ListenServer::getNbrServer())
		ListenServer::removeServers();
	ILogger::clearFiles();
	return(code);
}


/**

switch (status)
	{
		case RES_CONTINUE:
			client.setResponse(new SingleLineResponse(100, "Continue"));
			break;

		case RES_OK: //For Static/dir GET or CGI operation

				If cgi
					- file path
					- cgiConfig
					- method && headers
				If dir_listing
					- locationRule
				If static
					- file path

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
**/
