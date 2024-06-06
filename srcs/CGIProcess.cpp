#include <CGIProcess.hpp>
#include <fstream>
#include <iostream>
#include <Request.hpp>
#include <sys/time.h>
#include <ctime>

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

int CGIProcess::execCGI(Request &request)
{
    int pid;
    gettimeofday(&_fork_time, NULL);
    pid = fork();
    if (pid == 0)
    {
        _launchCGI(request);
    }
    else
    {
        _pid = pid;
        _status = CHILD_RUNNING;
        return (0);
    }
    return (0);
}

void    CGIProcess::_launchCGI(Request &request)
{
    (void) request;
    // set ENV variables with headers
    //dup stdout into tmp file
    //exec CGI
    // exit nicely in case of error
}
