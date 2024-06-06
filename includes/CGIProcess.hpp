#include <iostream>
#include <string>
#include <iterator>
#include <map>
#include <stdlib.h>

class Request;
typedef struct ResHints ResHints;

enum	CGI_RES_TYPE {CGI_RES_DOC = 0, CGI_RES_LOCAL_REDIRECT, CGI_RES_CLIENT_REDIRECT};
enum	CHILD_STATUS {CHILD_RUNNING, CHILD_TERM};



class CGIProcess {

	private:

		std::string							_line;
		size_t								_index;
		std::map<std::string, std::string> 	_cgi_headers;
		struct timeval						_fork_time;
		int									_pid;
		int									_status;

		int		_getLine(std::string &buffer);
		int		_extract_header();
		int		_inspectHeaders(ResHints &hints);
		int		_retrieveHeader(std::string key, std::string &value);
		void	_launchCGI(Request &request);
		void	_setVariables(Request &request);

	public:

		static char**						_env;
		/// @brief
		/// @return ```0``` if not finished, ```> 0``` if finished, ```< 0```
		/// if error.
		int	checkEnd() {return (0);}

		/// @brief Add potential header to resHints, change the status
		/// @param
		/// @return Identify document type
		int	parseHeaders(Request& request);
		int	execCGI(Request& request);
		int	getStatus();
		int	getPID();
		struct timeval	getForkTime() {return (_fork_time);}

		class	child_exit_exception: public std::exception
		{
			public:
				child_exit_exception() {}
				const char* what() const throw();
		};

};
