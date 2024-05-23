#ifndef REQUEST_HPP
#define REQUEST_HPP

#include <map>
#include <iterator>
#include <string>
#include <iostream>
#include <unistd.h>
#include <fstream>

#define HEADER_MAX_SIZE 4096
#define	HEADER_MAX_BUFFER 4

#define CRLF "\r\n"
#define SP " "

enum {none, chuncked, mesured};
enum {GET, POST, DELETE};
enum {COMPLETE, ONGOING, NEW};

std::string	generate_name(std::string &hostname);
bool nocase_string_eq(const std::string& a, const std::string& b);
int	getInt(std::string str, int base, int &res);

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

		int	_buffer_count;


		std::string							_line;

		std::string							_hostname;
		size_t								_body_max_size;
		int									_len;
		size_t								_content_length;
		bool								_chunked;
		std::string							_tmp_filename;
		std::fstream						_filestream;
		int									_b_status;
		bool								_chunked_status;



		int	_fillError(int error, std::string const &verbose);
		int	_parseChunked(std::string &buffer);
		int	_parseMesured(std::string &buffer);

	public:

		Request();
		~Request();

		void		addHeader(std::string const &name, std::string const &value);
		int			getLine(std::string &buffer);
		int			getChunkedSize(std::string &buffer);
		void		trimSpace();

		int			parseInput(std::string &buffer);
		int			parseRequestLine();
		int			parseHeaders(std::string &buffer);
		int			parseBody(std::string &buffer);
		int			getLenInfo();

		std::string	getHeader(std::string const &key);

		void	formatHeaders();
		void	printHeaders();
		void	setBodyMaxSize(size_t maxSize);

};



#endif
