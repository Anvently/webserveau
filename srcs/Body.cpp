#include <Body.hpp>


Body::Body()
{
}

Body::Body(Header *header, int max_size): _length(0), _max_body_size(max_size), _error_num(0), _header(header), _is_complete(0)
{
	std::string	str = header->getHeader("Content-Length");
	this->_chuncked = 0;
	if (nocase_string_eq(header->getHeader("Transfer-Encoding"), "chunked"))
	{
		this->_chuncked = 1;
		this->_content_length = -1;
	}
	else if (getInt(str, 10, this->_content_length))
		throw(Body::SizeRelatedHeaderMissing());

}

Body::SizeRelatedHeaderMissing::SizeRelatedHeaderMissing()
{}


const char *	Body::SizeRelatedHeaderMissing::what() const throw()
{
	return("Neither Content-Length nor Transfer-Encoding header is present.");
}


int	Body::_parseMesured(std::string &buffer)
{
	
}

int	Body::parseInput(std::string &buffer)
{
	if (this->_chuncked)
		this->_parseChunked(buffer);
	else
		this->_parseMesured(buffer);
}
