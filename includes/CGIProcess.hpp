#include <iostream>
#include <string>

class Request;

enum	CGI_RES_TYPE {CGI_RES_DOC = 0, CGI_RES_LOCAL_REDIRECT, CGI_RES_CLIENT_REDIRECT};

class CGIProcess {

	private:

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