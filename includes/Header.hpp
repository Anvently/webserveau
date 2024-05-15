#ifndef HEADER_HPP
#define HEADER_HPP

#include <map>
#include <iterator>
#include <string>

class Header{
	private:
		std::string							_method; //could be an int with an enum defined somewhere ?
		std::map<std::string, std::string>	_headers; //example <"Content type", "text/html">
		bool								_is_complete // set to one if "/r/n/r/n has been encoutered" ??
		//Should hold a pointer to the request/response it belongs to ? its fd ?

		//following attributes might refer to the last unfinished header treated (writen or read)
		std::string							_current_header;
		std::string::iterator				_current_it;

	public:

		Header();
		~Header();
		Header(Header& Copy);
		Header	&operator=(Header& Rhs);

		void	addHeader(std::string const &name, std::string const &value);
		void	writeHeader(int fd); //write the header into the right format into the given fd
		void	writeHeader(std::string &str); //write the headers into the right format into the given string
		bool	isComplete() const;

		// following function should parse the buffer and add the eventual headers and their values,
		//update current header and current iterator if not complete
		int		parseInput(std::string buffer);


};

#endif
