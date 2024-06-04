#ifndef RESPONSE_HPP
# define RESPONSE_HPP

#include <iostream>
#include <string>
#include <map>
#include <fstream>
#include <Request.hpp>
#include <queue>
#include <iterator>
#include <sstream>

// 1xx indicates an informational message only
// 2xx indicates success of some kind
// 3xx redirects the client to another URL
// 4xx indicates an error on the client's part
// 5xx indicates an error on the server's part
#define RES_CONTINUE 100
#define RES_OK 200
#define RES_CREATED 201
#define RES_NO_CONTENT 204
#define RES_MULTIPLE_CHOICE 300
#define RES_MOVED_PERMANENTLY 301
#define RES_FOUND 302
#define RES_SEE_OTHER 303
#define RES_TEMPORARY_REDIRECT 307
#define RES_BAD_REQUEST 400
#define RES_FORBIDDEN 403
#define RES_NOT_FOUND 404
#define RES_METHOD_NOT_ALLOWED 405
#define RES_TIMEOUT 408
#define RES_LENGTH_REQUIRED 411
#define RES_REQUEST_ENTITY_TOO_LARGE 413
#define RES_REQUEST_URI_TOO_LONG 414
#define RES_EXPECTATION_FAILED 417
#define RES_INTERNAL_ERROR 500
#define RES_NOT_IMPLEMENTED 501



/*
Response hints:
	- final path
	- was the ressource already existing ?
	- redirection location
	- allowed methods
	- verbose error

*/

/** @brief

Error response

**/
class	AResponse
{
	protected:

		AResponse(void) {}
		int									_status;
		std::string							_description;

	public:

		virtual ~AResponse() {}
		virtual int			writeResponse(std::queue<std::string>& outQueue) = 0;
		static AResponse	*genResponse(ResHints &hints);

};

class	SingleLineResponse : public AResponse
{
	private:

		SingleLineResponse(void);
		std::string	_response;

	public:

		virtual ~SingleLineResponse(void);
		SingleLineResponse(int status, const std::string& description);

		virtual int		writeResponse(std::queue<std::string>& outQueue);
};

class	HeaderResponse : public AResponse
{
	protected:

		HeaderResponse(void);
		std::map<std::string, std::string, i_less>	_headers;
		std::string									_formated_headers;

		void										_formatHeaders();

	public:

		HeaderResponse(int status, const std::string& description);
		virtual ~HeaderResponse(void);

		virtual int		writeResponse(std::queue<std::string>& outQueue);
		void	addHeader(std::string const &key, std::string const &value);
		void	addUniversalHeaders();
};

class	FileResponse : public HeaderResponse
{
	private:

		FileResponse();

		std::ifstream							_ifstream;
		std::string								_path;

	public:

		FileResponse(const std::string& infile, int status, const std::string &description);
		~FileResponse();

		virtual int		writeResponse(std::queue<std::string>& outQueue);
};

class	DynamicResponse : public HeaderResponse
{

	private:

		DynamicResponse(/* args */);
		int									_pipeFd;
		std::string							_body;

	public:

		// typedef	void (*bodyGenerator_func)(DynamicResponse&, const std::string);

		DynamicResponse(int status, const Request& request);
		// DynamicResponse(bodyGenerator_func, const std::string&);
		virtual ~DynamicResponse();

	virtual int		writeResponse(std::queue<std::string>& outQueue);
};


template <typename T>
  std::string itostr ( T num )
  {
     std::ostringstream ss;
     ss << num;
     return ss.str();
  }


#endif
