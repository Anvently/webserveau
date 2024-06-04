#include <Response.hpp>
#include <Request.hpp>
#include <sys/time.h>
#include <ctime>

static	std::string	days[] = {"Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"};
static	std::string	months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", }


SingleLineResponse::SingleLineResponse(int status, const std::string& description)
{
	_status = status;
	_description = description;
	_response = "HTTP/1.1 100 Continue\r\n";
}

int	SingleLineResponse::writeResponse(std::queue<char*>& outQueue)
{
	char	*str;
	std::string	portion;

	while(!_response.empty())
	{
		portion = _response.substr(0, HEADER_MAX_SIZE);
		str = new char[portion.size() + 1];
		std::copy(portion.begin(), portion.end(), str);
		str[portion.size()] = 0;
		outQueue.push(str);
		_response.erase(0, portion.size());
	}
	return (0);
}
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

void	HeaderResponse::addUniversalHeaders()
{
	_headers["Accept-Encoding"] = "identity";
	_headers["Accept-Ranges"] = "none";
	_headers["Date"] = getTime();
}
