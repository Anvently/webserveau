#include <Request.hpp>


Request::Request(): _method(-1), _error_num(0), _status(NEW), _buffer_count(0)
{
}

Request::~Request()
{}

void	Request::trimSpace()
{
	while (this->_line[0] == 32 || this->_line[0] == 9)
		this->_line.erase(0,1);
}

int	Request::_fillError(int error, std::string const &verbose)
{
	this->_error_num = error;
	this->_error_verbose = verbose;
	this->_line = "";
	return (error);
}

void	Request::addHeader(std::string const &name, std::string const &value)
{
	if (this->_headers[name] != "")
		this->_headers[name] += " ,";
	this->_headers[name] += value;
}

void	Request::formatHeaders()
{
	std::string	header;
	for (std::map<std::string, std::string>::iterator it = this->_headers.begin(); it != this->_headers.end(); it++)
	{
		header = it->first + ": " + it->second + "\r\n";
		this->_formated_headers += header;
		header = "";
	}
	this->_formated_headers += "\r\n";
}

int	Request::getLine(std::string &buffer)
{
	std::string::size_type	idx;

	idx = buffer.find(CRLF, 0);

	if (idx == std::string::npos)
	{
		this->_line	+= buffer;
		buffer = "";
		return (1);
	}

	this->_line += buffer.substr(0, idx);
	buffer = buffer.substr(idx + 2, std::string::npos);
	return (0);
}


static void	skipSpaces(std::string::size_type &idx, std::string &str)
{
	while ((str[idx] == 32 || str[idx] == 9) && idx < str.size())
		idx++;
}

static void	nextSpace(std::string::size_type &idx, std::string &str)
{
	while (str[idx] != 32 && str[idx] != 9 && idx < str.size())
		idx++;
}

static int	identifyMethod(std::string mtd)
{
	if (mtd == "GET")
		return (0);
	if (mtd == "POST")
		return (1);
	if (mtd == "DELETE")
		return (2);
	return (-1);
}

static	int	checkVersion(std::string &str)
{
	std::string::size_type idx = 0;
	if ((str.size() < 8) || str.find("HTTP/", idx) != 0)
		return (1);
	idx = 5;
	if (!isdigit(str[idx]))
		return (1);
	while (isdigit(str[idx]))
		idx++;
	if (idx == str.size())
		return (0);
	if (str[idx] != '.')
		return(1);
	idx++;
	if(!isdigit(str[idx]))
		return (1);
	while (isdigit(str[idx]))
		idx++;
	if (idx != str.size())
		return (1);
	return (0);
}

static	int	getHeaderName(std::string &line, std::string &header)
{
	std::string::size_type	idx = 0;

	idx = line.find(":", 0);
	if (idx == std::string::npos)
		return (1);
	header = line.substr(0,idx);
	line = line.substr(idx + 1, std::string::npos);
	return (0);
}

int	Request::parseRequestLine()
{
	std::string::size_type	idx = 0;
	std::string::size_type	r_idx = 0;
	std::string				version;
	nextSpace(idx, this->_line);
	if ((this->_method = identifyMethod(this->_line.substr(0, idx))) < 0)
		return (this->_fillError(405, "Method Not Allowd"));
	skipSpaces(idx, this->_line);
	r_idx = idx;
	nextSpace(r_idx, this->_line);
	this->_uri = std::string(this->_line, idx, r_idx - idx);
	version = this->_line.substr(r_idx + 1, std::string::npos);
	if (checkVersion(version))
		return (this->_fillError(400, "Request line incorrect syntax"));
	return(0);
}


int	Request::parseHeaders(std::string &buffer)
{
	std::string	header_name;
	std::string	header_value;

	this->_buffer_count++;
	if (this->_status == COMPLETE)
		return (0);
	if (this->_status == NEW)
	{
		while (!isalpha(buffer[0]))
			buffer.erase(0,1);
		if (this->getLine(buffer))
			return (0);
		if (this->_line.size() > HEADER_MAX_SIZE)
			return (this->_fillError(414, "Request uri too long"));
		if (this->parseRequestLine())
			return (this->_error_num);
		this->_line = "";
		this->_status = ONGOING;
	}
	while (!buffer.empty())
	{
		if (this->getLine(buffer))
			return (0);
		if (this->_line == "")
		{
			this->_status = COMPLETE;
			this->_line = "";
			return (0);
		}
		this->trimSpace();
		if (getHeaderName(this->_line, header_name))
			return (this->_fillError(400, "Invalid header line"));
		if (this->_line.size() > HEADER_MAX_SIZE)
			return (this->_fillError(431, "The " + header_name + " header was too large"));
		while (this->_line[0] == 32 || this->_line[0] == 9)
			this->_line.erase(0,1);
		header_value = this->_line;
		if (this->_headers[header_name] != "")
			this->_headers[header_name] += ", ";
		this->_headers[header_name] += header_value;
		this->_line = "";
	}
	if (this->_buffer_count == HEADER_MAX_BUFFER && this->_status != COMPLETE)
		return (this->_fillError(400, "Request header too long"));
	return (0);
}


int	Request::parseInput(std::string &buffer)
{
	if (this->_status != COMPLETE)
		this->parseHeaders(buffer);
	if (this->_error_num)
		return (this->_error_num);
	if (this->_status == COMPLETE && buffer != "")
		this->parseBody();

	//need to check for trailing headers.


}
