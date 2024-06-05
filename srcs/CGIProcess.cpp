#include <CGIProcess.hpp>
#include <fstream>
#include <iostream>
#include <Request.hpp>

int CGIProcess::parseHeaders(Request &request)
{
    std::fstream    fstream(request._resHints.path);
    char            *c_buffer = new char[HEADER_MAX_SIZE];;
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
    int res = _inspectHeaders(); // res should give the type of response document [local] redirect
    _updateHints(request._resHints);
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