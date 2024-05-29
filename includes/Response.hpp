#ifndef RESPONSE_HPP
# define RESPONSE_HPP

#include <iostream>
#include <string>
#include <map>
#include <fstream>

// 1xx indicates an informational message only
// 2xx indicates success of some kind
// 3xx redirects the client to another URL
// 4xx indicates an error on the client's part
// 5xx indicates an error on the server's part
#define RES_CONTINUE 100
#define RES_OK 200
#define RES_BAD_REQUEST 400
#define RES_NOT_FOUND 404
#define RES_REQUEST_ENTITY_TOO_LARGE 413
#define RES_INTERNAL_ERROR 500
#define RES_NOT_IMPLEMENTED 501

/** @brief 

Error response

**/
class	AResponse
{
	protected:


		int									_status;
		std::string							_description;

	public:

		virtual ~AResponse() = 0;
		virtual int			writeResponse(std::queue<char*>& outQueue) = 0;

};

class	SingleLineResponse : public AResponse
{
	private:

		SingleLineResponse(void);

	public:

		~SingleLineResponse(void);
		SingleLineResponse(int status, const std::string& description);

		virtual int		writeResponse(std::queue<char*>& outQueue);
};

class	EmptyBodyResponse : public AResponse
{
	private:

		EmptyBodyResponse(void) {}
		std::map<std::string, std::string>	_headers;

	public:

		EmptyBodyResponse(int status, const std::string& description) {}
		~EmptyBodyResponse(void) {}

		virtual int		writeResponse(std::queue<char*>& outQueue) {}
};

class	StaticPageResponse : public AResponse
{
	private:

		std::map<std::string, std::string>	_headers;
		std::ifstream						_ifstream;

	public:
		StaticPageResponse(/* args */);
		~StaticPageResponse();

		virtual int		writeResponse(std::queue<char*>& outQueue) {}
};

class	CGIResponse : public AResponse
{
	private:

		std::map<std::string, std::string>	_headers;
		int									_pipeFd;

	public:

		CGIResponse(/* args */);
		~CGIResponse();

		virtual int		writeResponse(std::queue<char*>& outQueue) {}
};

class	DynamicReponse : public AResponse
{

	private:

		DynamicReponse(/* args */);
		std::map<std::string, std::string>	_headers;
		int									_pipeFd;
		std::string							_body;

	public:

		typedef	void (*bodyGenerator_func)(DynamicReponse&, const std::string);
		
		DynamicReponse(bodyGenerator_func, const std::string&);
		~DynamicReponse();

	virtual int		writeResponse(std::queue<char*>& outQueue) {}
};

#endif