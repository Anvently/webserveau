#ifndef REQUEST_HPP
#define REQUEST_HPP

#include <map>
#include <iterator>
#include <string>
#include <iostream>
#include <unistd.h>
#include <fstream>
#include <ListenServer.hpp>


#define HEADER_MAX_SIZE 4096
#define	HEADER_MAX_BUFFER 4

#define CRLF "\r\n"
#define SP " "

enum {none, chuncked, mesured};
enum {GET, POST, DELETE};
enum {COMPLETE, ONGOING, NEW};

static std::string METHOD_STR[] = {"GET", "POST", "DELETE"};
#define METHOD_NBR 3
#define METHOD_IS_INVALID (&METHODS_STR[METHODS_NBR])

std::string	generate_name(std::string &hostname);
bool nocase_string_eq(const std::string& a, const std::string& b);
int	getInt(std::string str, int base, int &res);
int	getMethodIndex(const std::string& method);


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

		int	_header_size;


		std::string							_line;

		std::string							_hostname;
		int									_body_max_size;
		int									_len;
		int									_content_length;
		bool								_chunked;
		std::string							_tmp_filename;
		std::fstream						_filestream;
		int									_b_status;
		int									_chunked_body_size;
		bool								_chunked_status;
		int									_trailer_status;
		int									_trailer_size;

		int									_final_status;




		int	_checkSizes();
		int	_parseChunked(std::string &buffer);
		int	_parseMesured(std::string &buffer);

	public:

		Request();
		Request(const Request& copy);
		~Request();

		void		addHeader(std::string const &name, std::string const &value);
		int			getLine(std::string &buffer);
		int			getChunkedSize(std::string &buffer);
		void		trimSpace();

		int	_fillError(int error, std::string const &verbose);

		int			parseInput(std::string &buffer);
		int			parseRequestLine();
		int			parseHeaders(std::string &buffer);
		int			parseTrailerHeaders(std::string &buffer);
		int			parseBody(std::string &buffer);
		int			getLenInfo();
		int			getStatus(void) const;
		void		setStatus(int status);
		int			getError() const;

		std::string	getHeader(std::string const &key);
		int			getHostName(std::string &hostname);

		void	formatHeaders();
		void	printHeaders();
		void	setBodyMaxSize(int size);

		//DEBUG
		void	printRequest() const;

};



#endif
