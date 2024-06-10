#include <iostream>
#include <string>
#include <iterator>
#include <map>
#include <stdlib.h>

class Client;
class Request;
typedef struct ResHints ResHints;

enum	CGI_RES_TYPE {CGI_RES_DOC = 0, CGI_RES_LOCAL_REDIRECT, CGI_RES_CLIENT_REDIRECT};
enum	CHILD_STATUS {CHILD_RUNNING, CHILD_TERM};

std::string	IntToString(int x, int base);

class CGIProcess {

	private:

		CGIProcess();

		std::string							_line;
		size_t								_index;
		std::map<std::string, std::string> 	_cgi_headers;
		struct timeval						_fork_time;
		int									_pid;
		int									_status;
		Request&							_request;
		Client&								_client;

		int		_getLine(std::string &buffer);
		int		_extract_header();
		int		_inspectHeaders();
		int		_retrieveHeader(std::string key, std::string &value);
		void	_launchCGI();
		void	_setVariables();

	public:

		CGIProcess(Client& client);

		static char**						_env;
		/// @brief
		/// @return ```0``` if not finished, ```> 0``` if finished, ```< 0```
		/// if error.
		int	checkEnd() {return (0);}

		/// @brief Add potential header to resHints, change the status
		/// @param
		/// @return Identify document type
		int	parseHeaders();
		int	execCGI();
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
