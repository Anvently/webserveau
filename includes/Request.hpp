#ifndef REQUEST_HPP
#define REQUEST_HPP

#include <map>
#include <iterator>
#include <string>
#include <iostream>

#define HEADER_MAX_SIZE 4096
#define	HEADER_MAX_BUFFER 4

#define CRLF "\r\n"
#define SP " "

enum {none, chuncked, mesured};
enum {GET, POST, DELETE};
enum {COMPLETE, ONGOING, NEW};

typedef	struct body
{
	std::string	content;
	int			size;
	int			expected_size;
	bool		chuncked;
} body;


class	Request
{
	private:

		std::map<std::string, std::string>	_headers;
		std::string							_formated_headers;
		std::string							_uri;
		int									_method;
		int									_error_num;
		std::string							_error_verbose;
		int									_status;

		std::string							_line;

		int	_buffer_count;

		int	_fillError(int error, std::string const &verbose);

	public:

		Request();
		~Request();

		void		addHeader(std::string const &name, std::string const &value);
		int			getLine(std::string &buffer);
		void		trimSpace();

		int			parseInput(std::string &buffer);
		int			parseRequestLine();
		int			parseHeaders(std::string &buffer);
		int			parseBody();

		std::string	getHeader() const;

		void	formatHeaders();
		void	printHeaders();

};



#endif
