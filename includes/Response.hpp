#ifndef RESPONSE_HPP
# define RESPONSE_HPP

#include <iostream>
#include <string>
#include <map>
#include <fstream>

/** @brief 

Error response

**/
class	AResponse
{
	protected:

		virtual ~AResponse() = 0;

		int									_status;
		std::string							_description;

	public:

		virtual int			writeResponse(std::queue<char*>& outQueue) = 0;

};

class	SingleLineResponse : AResponse
{
	private:

		SingleLineResponse(void);

	public:

		~SingleLineResponse(void);
		SingleLineResponse(int status, const std::string& description);

		virtual int		writeResponse(std::queue<char*>& outQueue);
};

class	EmptyBodyResponse : AResponse
{
	private:

		EmptyBodyResponse(void) {}
		std::map<std::string, std::string>	_headers;

	public:

		EmptyBodyResponse(int status, const std::string& description) {}
		~EmptyBodyResponse(void) {}

		virtual int		writeResponse(std::queue<char*>& outQueue) {}
};

class	StaticPageResponse : AResponse
{
	private:

		std::map<std::string, std::string>	_headers;
		std::ifstream						_ifstream;

	public:
		StaticPageResponse(/* args */);
		~StaticPageResponse();

		virtual int		writeResponse(std::queue<char*>& outQueue) {}
};

class	CGIResponse : AResponse
{
	private:

		std::map<std::string, std::string>	_headers;
		int									_pipeFd;

	public:

		CGIResponse(/* args */);
		~CGIResponse();

		virtual int		writeResponse(std::queue<char*>& outQueue) {}
};

class	DirListingResponse : AResponse
{
	private:

		std::map<std::string, std::string>	_headers;
		int									_pipeFd;

	public:
		DirListingResponse(/* args */);
		~DirListingResponse();

	virtual int		writeResponse(std::queue<char*>& outQueue) {}
};

#endif