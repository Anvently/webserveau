#include <Header.hpp>
#include <algorithm>

Header::Header(): _method(-1), _buffer_count(0), _error_num(0), _status(NEW)
{
}

Header::~Header()
{}


void	Header::addHeader(std::string const &name, std::string const &value)
{
	this->_headers[name] = value;
}

void	Header::formatHeaders()
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

// parse the buffer to extract the next line. return 0 on success, 1 if the buffer ended without a line
// also update the buffer removing the line extracted
int	Header::getLine(std::string &buffer)
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
	if (buffer[0] == 32 || buffer[0] == 9)
	{
		while (buffer[0] == 32 || buffer[0] == 9)
			buffer.erase(0, 1);
	}
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

int	Header::_fillError(int error, std::string const &verbose)
{
	this->_error_num = error;
	this->_error_verbose = verbose;
	this->_line = "";
	return (error);
}

// Where to add the error in case of too long header (4 buffer already parsed ?) here or in the request obj ?

int	Header::parseInput(std::string &buffer)
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
		if (this->parseFirstLine())
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
	return (0);
}


void	Header::printLine()
{
	std::cout << this->_line;
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

int	Header::parseFirstLine()
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


int	Header::getStatus() const
{
	return (this->_status);
}

void	Header::printHeaders(void) const
{
	std::cout << this->_formated_headers;
}

void	Header::printRequest() const
{
	std::cout << "method is: " << this->_method << std::endl;
	std::cout << "uri is: " << this->_uri << std::endl;
	std::cout << "status is: " << this->getStatus() << std::endl;
	std::cout << "error is: " << this->_error_num << std::endl;
}


std::string	Header::getHeader(std::string const &key)
{
	return (this->_headers[key]);
}

int	Header::getMethodIndex(const std::string& method)
{
	for (int i = 0 ; i < METHOD_NBR; i++)
	{
		if (METHOD_STR[i] == method)
			return (i);
	}
	return (-1);
}
