#include <Response.hpp>
#include <Request.hpp>
#include <sys/time.h>
#include <fstream>
#include <ctime>

static	std::string	days[] = {"Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"};
static	std::string	months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", };


std::map<int, std::string>	init_response()
{
	std::map<int, std::string>	mipmap;

	mipmap.insert({100, "Continue"});
	mipmap.insert({200, "OK"});
	mipmap.insert({201, "CREATED"});
	mipmap.insert({204, "No Content"});
	mipmap.insert({300, "Multiple Choices"});
	mipmap.insert({301, "Move Permanently"});
	mipmap.insert({302, "Found"});
	mipmap.insert({307, "Temporary Redirect"});



}

static std::map<int, std::string> ResponseLine = init_response();
// {{100, "Continue"}, {200, "OK"}, {201, "Created"}, {204, "No Content"}, \
// {300, "Multiple Choices"}, {301, "Move Permanently"}, {302, "Found"}, {307, "Temporary Redirect"}, \
// {308, "Permanent Redirect"}, {400, "Bad Request"}, {401, "Unauthorized"}, {403, "Forbidden"}, {404, "Not Found"}, \
// {405, "Method Not Allowed"}, {408, "Request Timeout"}, {413, "Content Too Large"}, {414, "URI Too Long"}, \
// {415, "Unsupported Media Type"}, {417, "Expectation Failed"}, {500, "Internal Server Error"}, {501, "Not Implemented"}, \
// {505, "HTTP Version Not Supported"}};


SingleLineResponse::SingleLineResponse(int status, const std::string& description)
{
	_status = status;
	_description = description;
	_response = "HTTP/1.1 100 Continue\r\n";
}

int	SingleLineResponse::writeResponse(std::queue<std::string>& outQueue)
{
	std::string	portion;

	while(!_response.empty())
	{
		portion = _response.substr(0, HEADER_MAX_SIZE);
		outQueue.push(portion);
		_response.erase(0, portion.size());
		portion.clear();
	}
	return (0);
}

SingleLineResponse::~SingleLineResponse(void) {}
HeaderResponse::HeaderResponse(int status, const std::string& description)
{
	_status = status;
	_description = description;
	addUniversalHeaders();
}

std::string	getTime()
{
	time_t	t = time(0);
	struct tm *ptime = gmtime(&t);
	char	str[30];
	strftime(str, 30, "%a, %d %b %G %T GMT", ptime);
	std::string	time(str);
	return (time);
}

HeaderResponse::HeaderResponse(void) {}

HeaderResponse::~HeaderResponse(void) {}

void	HeaderResponse::addUniversalHeaders()
{
	_headers["Accept-Encoding"] = "identity";
	_headers["Accept-Ranges"] = "none";
	_headers["Date"] = getTime();
}

void	HeaderResponse::addHeader(std::string const &key, std::string const &value)
{
	_headers[key] = value;
}

void	HeaderResponse::_formatHeaders()
{
	_formated_headers += "HTTP/1.1" + ResponseLine[_status] + "\r\n";
	for (std::map<std::string, std::string>::iterator it = _headers.begin(); it != _headers.end(); it++)
	{
		_formated_headers += it->first + ": ";
		_formated_headers += it->second + "\r\n";
	}
	_formated_headers += "\r\n";
}

int	HeaderResponse::writeResponse(std::queue<std::string>& outQueue)
{
	std::string	portion;

	_formatHeaders();
	while(!_formated_headers.empty())
	{
		portion = _formated_headers.substr(0, HEADER_MAX_SIZE);
		outQueue.push(portion);
		_formated_headers.erase(0, portion.size());
		portion.clear();
	}
	return (0);
}

AResponse*	AResponse::genResponse(ResHints& hints) {
	(void)hints;
	return (NULL);
}

FileResponse::~FileResponse(void) {}


FileResponse::FileResponse(const std::string& infile, int status, const std::string &description): HeaderResponse(status, description)
{
	_path = infile;
	_ifstream.open(_path.c_str(), std::ios_base::in | std::ios_base::binary); // le fichier devrait tjrs etre ouvrable ?
}

int	FileResponse::writeResponse(std::queue<std::string>& outQueue)
{
	(void) outQueue;
	return (0);
}

DynamicResponse::DynamicResponse(int status, const Request& request) {
	(void) status;
	(void) request;
}

DynamicResponse::~DynamicResponse(void) {}

int	DynamicResponse::writeResponse(std::queue<std::string>& outQueue) {
	(void)outQueue;
	return (0);
}

AResponse*	AResponse::genResponse(ResHints &hints)
{
	AResponse	*response;
	if (hints.status == 100)
	{
		response = new SingleLineResponse(hints.status, )
	}

}
