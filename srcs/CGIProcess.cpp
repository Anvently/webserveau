#include <CGIProcess.hpp>
#include <fstream>
#include <iostream>
#include <Request.hpp>
#include <sys/time.h>
#include <ctime>
#include <sys/wait.h>
#include <IControl.hpp>

char**  CGIProcess::_env;

CGIProcess::CGIProcess(Client& client) : _client(client), _request(*client.getRequest()) {}

CGIProcess::~CGIProcess(void) {
	unlink(_request.resHints.path.c_str());
}

int CGIProcess::parseHeaders()
{
	std::fstream    fstream(_request.resHints.path.c_str());
	char            *c_buffer = new char[HEADER_MAX_SIZE];
	fstream.read(c_buffer, HEADER_MAX_SIZE);
	std::string     buffer(c_buffer, fstream.gcount());

	while(_getLine(buffer))
	{
		if (_line.empty())
			break;
		if (_extract_header())
		{
			_request.resHints.status = 500;
			//do i need to unling now?
			return (0);
		}
	}
	_request.resHints.index = _index;
	int res = _inspectHeaders(); // res should give the type of response document [local] redirect
	return (res); // res should allow to discriminate what to do with the request
}

int CGIProcess::_getLine(std::string &buffer)
{
	size_t  idx = buffer.find("\r\n", 0);
	if (idx == std::string::npos)
		return (0);
	_line = buffer.substr(0, idx);
	_index += idx + 2;
	buffer = buffer.substr(idx + 2, std::string::npos);
	return (1);
}

int CGIProcess::_extract_header()
{
	std::string key;
	std::string value;
	size_t  idx = _line.find(":", 0);
	if (idx == std::string::npos)
		return (500);
	key = _line.substr(0, idx);
	value = _line.substr(idx + 1, _line.size());
	_cgi_headers[key] = value;
	_line.clear();
	return (0);
}

int CGIProcess::_retrieveHeader(std::string key, std::string &value)
{
	try{
		value = _cgi_headers.at(key);
		return (1);
	}
	catch(std::out_of_range &e)
	{
		return (0);
	}
}


int CGIProcess::_inspectHeaders()
{
	ResHints&   hints = _request.resHints;
	std::string value;
	if (_retrieveHeader("Location", value) && value.substr(0, 7) == "http://")
	{
		hints.headers["Location"] = value;
		hints.status = 302;
		if (_retrieveHeader("Content-Type", value))
		{
			hints.headers["Content-Type"] = value;
			hints.hasBody = 1;
		}
		return (CGI_RES_CLIENT_REDIRECT);
	}
	else if (_retrieveHeader("Location", value) && value[0] == '/')
	{
		hints.redir_type = REDIR_LOCAL;
		hints.path = value;
		return (CGI_RES_LOCAL_REDIRECT);
	}
	else
	{
		if (_retrieveHeader("Status", value) && getInt(value, 10, hints.status))
		{
			hints.status = 500;
			return (CGI_RES_DOC);
		}
		else
			hints.status = 200;
		if (_retrieveHeader("Content-Type", value))
		{
			hints.hasBody = 1;
			hints.headers["Content-Type"] = value;
		}
		else
			hints.hasBody = 0;
	}
	return (CGI_RES_DOC);
}

static long long	getDuration(struct timeval time)
{
	struct timeval	now;
	gettimeofday(&now, NULL);

	return (now.tv_sec * 1000 + now.tv_usec / 1000 - time.tv_sec * 1000 - time.tv_usec / 1000);
}

int	CGIProcess::checkEnd() {
	int	status;

	if (_pid == 0)
		return (-1);
	if (waitpid(_pid, &status, WNOHANG) == 0) {
		if (getDuration(_fork_time) > CGI_TIME_OUT) {
			kill(_pid, SIGKILL);
			return (-1);
		}
		return (0);
	}
	if (WEXITSTATUS(status) != 0 || WIFSIGNALED(status))
		return (-1);
	return (1);
}

/// @brief 
/// @return ```0``` for success. ```-1``` if error. Child error will be handled
/// elsewhere
int CGIProcess::execCGI()
{
	int pid;
	gettimeofday(&_fork_time, NULL);
	_request.resHints.path = generate_name(NULL);
	pid = fork();
	if (pid == 0)
	{
		_launchCGI();
	}
	else if (pid > 0)
	{
		_pid = pid;
		_status = CHILD_RUNNING;
		return (0);
	} else if (pid < 0)
		return (1);
	return (0);
}

void    CGIProcess::_launchCGI()
{
	int     fd_out = open(_request.resHints.path.c_str(), O_RDWR | O_TRUNC | O_CREAT, 0644);
	int     fd_in;
	char    **argv = new char*[3];
	argv[2] = NULL;
	argv[0] = new char[_request.resHints.cgiRules->exec.size() + 1];
	argv[0][_request.resHints.cgiRules->exec.size()] = '\0';
	argv[1] = new char[_request.resHints.scriptPath.size() + 1];
	argv[1][_request.resHints.scriptPath.size()] = 0;
	std::copy(&_request.resHints.cgiRules->exec.c_str()[0], &_request.resHints.cgiRules->exec.c_str()[_request.resHints.cgiRules->exec.size()], &argv[0][0]);
	std::copy(&_request.resHints.scriptPath.c_str()[0], &_request.resHints.scriptPath.c_str()[_request.resHints.scriptPath.size()], &argv[1][0]);
	if (!_request.resHints.bodyFileName.empty())
	{
		fd_in = open(_request.resHints.bodyFileName.c_str(), O_RDONLY);
		if (dup2(fd_in, STDIN_FILENO) < 0)
			LOGE("Dup2() with infile (%ss) failed", &_request.resHints.bodyFileName);
		close(fd_in);
	}
	if (dup2(fd_out, STDOUT_FILENO) < 0)
		LOGE("Dup2() with outfile (%ss) failed", &_request.resHints.path);
	close(fd_out);
	_setVariables();
	execv(argv[0], argv);
	delete[] argv[0];
	delete[] argv[1];
	delete[] argv;
	LOGE("The script %ss failed", &_request.resHints.scriptPath);
	throw(CGIProcess::child_exit_exception());
}

int CGIProcess::getStatus()
{
	return (_status);
}

int CGIProcess::getPID()
{
	return (_pid);
}

const char* CGIProcess::child_exit_exception::what() const throw() {

	return ("Script failed");
}

void    CGIProcess::_setVariables()
{
	std::string         str;

	setenv("REQUEST_METHOD", METHOD_STR[_request.method].c_str(), 1);
	setenv("QUERY_STRING", _request.parsedUri.query.c_str(), 1);

	setenv("GATEWAY_INTERFACE", "CGI/1.1", 1); // which version to use ?
	setenv("SERVER_PROTOCOL", "HTTP/1.1", 1);
	setenv("SERVER_NAME", _request.getHeader("Host").c_str(), 1);
	setenv("REMOTE_ADDR", _client.getStrAddr().c_str(), 1);
	setenv("SERVER_PORT", IntToString(_client.getAddrPort(), 10).c_str(), 1);

	if (!_request.parsedUri.pathInfo.empty())
	{
		setenv("PATH_INFO", _request.parsedUri.pathInfo.c_str(), 1);
		str = _request.resHints.cgiRules->root + _request.parsedUri.pathInfo;
		setenv("PATH_TRANSLATED", str.c_str() , 1); //TODODODODODODO
	}
	else
	{
		unsetenv("PATH_INFO");
		unsetenv("PATH_TRANSLATED");
		// setenv("PATH_INFO", NULL, 1);
		// setenv("PATH_TRANSLATED", NULL, 1);
	}
	setenv("SCRIPT_NAME", _request.resHints.scriptPath.c_str(), 1); //Not sure which variable to use
	if (_request.resHints.hasBody && _retrieveHeader("Content-Type", str))
		setenv("CONTENT_TYPE", str.c_str(), 1);
	else
		unsetenv("CONTENT_TYPE");


}


/*
CGI ENV variables:

CONTENT_TYPE -> if the input includes a body this is the content-type of the request
GATEWAY_INTERFACE -> CGI/1.1
PATH_INFO -> the path following the cgi in the uri
PATH_TRANSLATED -> example http://host:port/path/to/script.cgi/path/info
					-> path info = /path/info
					->path_translated = /root/path/info
		IF path_info = NULL => path_translated=NULL

QUERY STRING -> the query string, if not query set it as ""

REMOTE_ADDR = the IP of the client ????
REMOTE_HOST = domain name of the client

REQUEST_METHOD -> get, post, delete (case sensitive)
SCRIPT_NAME -> should identify the cgi script
SERVER_NAME -> name of the server host dealing with the request (==Host: header)\
SERVER_PORT -> ...
SERVER_PROTOCOL -> HTTP/1.1









*/
