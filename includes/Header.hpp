#ifndef HEADER_HPP
#define HEADER_HPP

#include <map>
#include <iterator>
#include <string>
#include <iostream>

#define HEADER_MAX_SIZE 4096
#define	HEADER_MAX_BUFFER 4

#define CRLF "\r\n"
#define SP " "


enum {GET, POST, DELETE};
enum {COMPLETE, ONGOING, NEW};


class Header{
	private:
		int									_method; // -1 if the method is invalid/unknown
		std::string							_uri;
		std::map<std::string, std::string>	_headers; //example <"Content type", "text/html">
		std::string							_formated_headers;
		//Should hold a pointer to the request/response it belongs to ? its fd ?

		std::string							_line;
		int									_buffer_count;
		int									_error_num;
		std::string							_error_verbose;

		//following attributes might refer to the last unfinished io (write or read)
		int	_status; // see enum abov
		std::string::iterator				_current_it; // where to restart write/read on the next epoll
		std::string							_current_header;
		std::string							_current_value;

		int	_fillError(int error, std::string const &verbose);



	public:

		Header();
		~Header();
		Header(Header& Copy);
		Header	&operator=(Header& Rhs);

		void	addHeader(std::string const &name, std::string const &value);
		void	formatHeaders();
		int		writeHeader(int fd); //write the headers into the right format into the given fd
		int		writeHeader(std::string &str); //write the headers into the right format into the given string
		bool	isComplete() const;

		// following function should parse the buffer and add the eventual headers and their values,
		//update current header and current iterator if not complete
		int		parseInput(std::string &buffer);
		int		parseLine();
		int		getLine(std::string &buffer);
		int		parseFirstLine();
		std::string	getHeader(std::string const &key);

		void	printLine();
		void	printHeaders() const;
		void	printRequest() const;
		int		getStatus() const;


};

#endif
