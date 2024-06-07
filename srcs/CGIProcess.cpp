#include <CGIProcess.hpp>
#include <fstream>
#include <iostream>
#include <Request.hpp>
#include <sys/time.h>
#include <ctime>
#include <IControl.hpp>

int CGIProcess::parseHeaders(Request &request)
{
    std::fstream    fstream(request._resHints.path.c_str());
    char            *c_buffer = new char[HEADER_MAX_SIZE];
    fstream.read(c_buffer, HEADER_MAX_SIZE);
    std::string     buffer(c_buffer, fstream.gcount());

    while(_getLine(buffer))
    {
        if (_line.empty())
            break;
        if (_extract_header())
        {
            request._resHints.status = 500;
            //do i need to unling now?
            return (0);
        }
    }
    request._resHints.index = _index;
    int res = _inspectHeaders(request._resHints); // res should give the type of response document [local] redirect
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


int CGIProcess::_inspectHeaders(ResHints &hints)
{
    std::string value;
    if (_retrieveHeader("Location", value) && value.substr(0, 7) == "http://")
    {
        hints.redir_type = REDIR_CLIENT;
        hints.headers["Location"] = value;
        hints.status = 302;
        if (_retrieveHeader("Content-Type", value))
        {
            hints.headers["Content-Type"] = value;
            hints.hasBody = 1;
        }
        return (0);
    }
    else if (_retrieveHeader("Location", value) && value[0] == '/')
    {
        hints.redir_type = REDIR_LOCAL;
        hints.path = value;
        return (1);
    }
    else
    {
        if (_retrieveHeader("Status", value) && getInt(value, 10, hints.status))
        {
            hints.status = 500;
            return (0);
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
    return (0);
}

int CGIProcess::execCGI(Client &client)
{
    int pid;
    gettimeofday(&_fork_time, NULL);
    client.getRequest()->_resHints.cgiOutput = generate_name(client.getRequest()->_resHints.path);
    pid = fork();
    if (pid == 0)
    {
        _launchCGI(client);
    }
    else
    {
        _pid = pid;
        _status = CHILD_RUNNING;
        return (0);
    }
    return (0);
}

void    CGIProcess::_launchCGI(Client &client)
{
    int fd_out = open(client.getRequest()->_resHints.cgiOutput.c_str(), O_CREAT | O_TRUNC);
    int fd_in;
    if (!client.getRequest()->_resHints.bodyFileName.empty())
    {
        fd_in = open(client.getRequest()->_resHints.bodyFileName.c_str(), O_RDONLY);
        dup2(fd_in, STDIN_FILENO);
    }
    dup2(fd_out, STDOUT_FILENO);
    _setVariables(client);
    //setup env variables
    //dup stdin into the body input file ?
    //exec CGI => script path in hints
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

void    CGIProcess::_setVariables(Client &client)
{
    Request request = *client.getRequest();

    setenv("REQUEST_METHOD", METHOD_STR[request._method].c_str(), 1);
    setenv("QUERY_STRING", request._parsedUri.query.c_str(), 1);

    setenv("GATEWAY_INTERFACE", "CGI/1.1", 1); // which version to use ?
    setenv("SERVER_PROTOCOL", "HTTP/1.1", 1);
    setenv("SERVER_NAME", request.getHeader("Host").c_str(), 1);
    setenv("REMOTE_ADDR", client.getStrAddr().c_str(), 1);
    setenv("SERVER_PORT", IntToString(client.getAddrPort(), 10).c_str(), 1);

    if (!request._parsedUri.pathInfo().empty())
    {
        setenv("PATH_INFO", request._parsedUri.pathInfo.c_str(), 1);
        setenv("PATH_TRANSLATED", "", 1); //TODODODODODODO
    }
    else
    {
        setenv("PATH_INFO", NULL, 1);
        setenv("PATH_TRANSLATED", NULL, 1);
    }
    setenv("SCRIPT_NAME", request._parsedUri.path.c_str(), 1); //Not sure which variable to use

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
