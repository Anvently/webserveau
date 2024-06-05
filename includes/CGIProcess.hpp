#include <iostream>
#include <string>
#include <iterator>

class Request;

enum	CGI_RES_TYPE {CGI_RES_DOC = 0, CGI_RES_LOCAL_REDIRECT, CGI_RES_CLIENT_REDIRECT};

class CGIProcess {

	private:

		std::string	_line;
		size_t		_index;
		std::map<std::string, std::string> _cgi_headers;

		int	_getLine(std::string &buffer);
		int	_extract_header();

	public:

		/// @brief
		/// @return ```0``` if not finished, ```> 0``` if finished, ```< 0```
		/// if error.
		int	checkEnd() {return (0);}

		/// @brief Add potential header to resHints, change the status
		/// @param 
		/// @return Identify document type
		int	parseHeaders(Request&) {return (0);}

};