#include <Request.hpp>


Request::Request(): _method(-1), _error_num(0), _status(NEW), _buffer_count(0), _len(0), _content_length(-1), _chunked(0), _b_status(NEW), _chunked_status(0)
{
}

Request::~Request()
{}

void	Request::trimSpace()
{
	while (this->_line[0] == 32 || this->_line[0] == 9)
		this->_line.erase(0,1);
}

std::string	Request::getHeader(std::string const &key)
{
	return (this->_headers[key]);
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

int	Request::_parseMesured(std::string &buffer)
{
	size_t	left_to_complete;
	size_t	cut;
	//size_t	n_writen;
	std::string	extract;

	left_to_complete = this->_content_length - this->_len;
	if (left_to_complete > buffer.size())
		cut = buffer.size();
	else
		cut = left_to_complete;
	extract = buffer.substr(0, cut);
	this->_filestream << extract;
	buffer = buffer.substr(cut, std::string::npos);
	if (this->_len == this->_content_length)
	{
		this->_b_status = COMPLETE;
		this->_filestream.close();
	}
	return (0);
}

int	Request::getLenInfo()
{
	std::string	str = this->getHeader("Content-Length");
	int	res = 0;
	if (nocase_string_eq(this->getHeader("Transfer-Encoding"), "chunked"))
	{
		this->_chunked = 1;
		this->_content_length = -1;
	}
	else if (this->getHeader("Content-Length") != "")
	{
		if (getInt(this->getHeader("Content-Length"), 10, res))
			return (this->_fillError(400, "Incorrect content-length provided"));
		this->_content_length = res;
	}
	else
		this->_b_status = COMPLETE;
	return (0);
}


int	Request::getChunkedSize(std::string &buffer)
{
	std::string::size_type	idx;
	int	len = 0;

	if (this->_chunked_status == 1)
		return (0);
	idx = buffer.find(CRLF, 0);
	if (idx == std::string::npos)
	{
		this->_line	+= buffer;
		buffer = "";
		return (0);
	}
	this->_line += buffer.substr(0, idx);
	if (getInt(this->_line, 16, len) || len < 0)
		return (this->_fillError(400, "Syntax error parsing chunked body"));
	std::cout << "len expected: " << len << std::endl;
	this->_len = len;
	this->_line = "";
	buffer = buffer.substr(idx + 2, std::string::npos);
	this->_chunked_status = 1;
	return (0);
}

int	Request::_parseChunked(std::string &buffer)
{
	if (this->_b_status == COMPLETE)
		return (0);
	while (buffer != "")
	{
		if (this->getChunkedSize(buffer))
			return (this->_error_num);
		if (this->_chunked_status == 1 && this->_len == 0)
		{
			this->_b_status = COMPLETE;
			this->_line = "";
			this->_filestream.close();
			return (0);
		}
		if (this->getLine(buffer))
			return (0);
		if (this->_line.size() != static_cast<unsigned long> (this->_len))
			return(this->_fillError(400, "Size error in chunked body"));
		std::cout << "trying to insert to body: " << this->_line << std::endl;
		this->_filestream << this->_line;
		this->_line = "";
		this->_len = -1;
		this->_chunked_status = 0;
	}
	return (0);
}

int	Request::parseBody(std::string &buffer)
{
	if (this->_b_status == COMPLETE)
		return (0);
	if (this->_b_status == NEW)
	{
		if (this->getLenInfo() || this->_b_status == COMPLETE)
			return (this->_error_num);
		this->_tmp_filename = generate_name(this->_hostname);
		this->_filestream.open(this->_tmp_filename.c_str(), std::ios::out | std::ios::app | std::ios::binary);
		this->_b_status = ONGOING;
	}
	if (this->_chunked)
	{
		while (!isalnum(buffer[0]))
			buffer.erase(0,1);
		return (this->_parseChunked(buffer));
	}
	else
		return (this->_parseMesured(buffer));

}

//Client should call the different parsing methods itself and set the body max size value ?
// int	Request::parseInput(std::string &buffer)
// {
// 	if (this->_status != COMPLETE)
// 		this->parseHeaders(buffer);
// 	if (this->_error_num)
// 		return (this->_error_num);
// 	if (this->_status == COMPLETE && buffer != "")
// 		this->parseBody(buffer);

// 	//need to check for trailing headers.


// }


void	Request::printRequest() const
{
	std::cout << "method is: " << this->_method << std::endl;
	std::cout << "uri is: " << this->_uri << std::endl;
	std::cout << "status is: " << this->_status << std::endl;
	std::cout << "bstatus is: " << this->_b_status << std::endl;
	std::cout << "error is: " << this->_error_num << std::endl;
}


void	Request::printHeaders()
{
	for(std::map<std::string, std::string>::iterator it = this->_headers.begin(); it != this->_headers.end(); it++)
	{
		std::cout << it->first << ": " << it->second << "\r\n";
	}
}
