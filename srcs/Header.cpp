#include <Header.hpp>

Header::Header(): _laststop(-1), _method(-1)
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
		header = it->first + ": " + it->second + "/r/n";
		this->_formated_headers += header;
		header = "";
	}
	this->_formated_headers += "/r/n";
}


