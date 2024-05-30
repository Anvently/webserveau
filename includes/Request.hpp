#ifndef REQUEST_HPP
#define REQUEST_HPP

#include <map>
#include <iterator>
#include <string>
#include <iostream>
#include <unistd.h>
#include <fstream>
#include <ListenServer.hpp>
#include <cstring>


#define HEADER_MAX_SIZE 4096
#define	HEADER_MAX_BUFFER 4

#define CRLF "\r\n"
#define SP " "

enum {none, chuncked, mesured};
enum {GET, POST, DELETE};
enum {COMPLETE, ONGOING, NEW};
enum {HEADER, HOST, BODY, TRAILER, CONT, COMPLETE, ERROR};

static std::string METHOD_STR[] = {"GET", "POST", "DELETE"};
#define METHOD_NBR 3
#define METHOD_IS_INVALID (&METHODS_STR[METHODS_NBR])

std::string	generate_name(std::string &hostname);
bool nocase_string_eq(const std::string& a, const std::string& b);
int	getInt(std::string str, int base, int &res);
int	getMethodIndex(const std::string& method);

typedef struct URI
{
	std::string	path;
	std::string	root;
	std::string	extension;
	std::map<std::string, std::string>	var;
}	URI;


struct i_less {
	static inline char	lowercase(char c) {
		if (c >= 'A' || c <= 'Z')
			return (c + ('a' - 'A'));
		return (c);
	};
	static inline int	stricmp(const char* s1, const char* s2) {
		while (*s1 && (lowercase(*s1) == lowercase(*s2)))
		{
			s1++;
			s2++;
		}
		return ((unsigned char) *s1 - (unsigned char) *s2);
	};
	bool operator() (const std::string& lhs, const std::string& rhs) const {
		return stricmp(lhs.c_str(), rhs.c_str()) < 0;
	}
};

class	Request
{
	private:

		std::map<std::string, std::string, i_less>	_headers;
		std::string							_formated_headers;
		std::string							_uri;
		int									_method;
		int									_error_num;
		std::string							_error_verbose;
		int									_status;

		int									_header_size;


		std::string							_line;

		std::string							_hostname;
		int									_body_max_size;
		int									_len;
		int									_content_length;
		bool								_chunked;
		int									_b_status;
		int									_chunked_body_size;
		bool								_chunked_status;
		int									_trailer_status;
		int									_trailer_size;

		int									_final_status;




		int	_checkSizes();
		int	_parseChunked(std::string &buffer, std::fstream *filestream);
		int	_parseMesured(std::string &buffer, std::fstream *filestream);

	public:

		Request();
		Request(const Request& copy);
		~Request();

		void		addHeader(std::string const &name, std::string const &value);
		int			getLine(std::string &buffer);
		int			getChunkedSize(std::string &buffer);
		void		trimSpace();

		int			_fillError(int error, std::string const &verbose);

		int			parseInput(std::string &buffer, std::fstream *filestream);
		int			parseRequestLine();
		int			parseHeaders(std::string &buffer);
		int			parseTrailerHeaders(std::string &buffer);
		int			parseBody(std::string &buffer, std::fstream *filestream);
		int			getLenInfo();
		int			getStatus(void) const;
		void		setStatus(int status);
		int			getError() const;

		const std::string&	getHeader(std::string const &key) const;
		int					getHostName(std::string &hostname) const;
		bool				checkHeader(const std::string& key) const;

		void	formatHeaders();
		void	printHeaders();
		void	setBodyMaxSize(int size);

		int		parseURI();

		//DEBUG
		void	printRequest() const;

};



#endif
