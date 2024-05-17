#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <IObject.hpp>

class Request;
class Response;
class Host;


class	Client : public IObject
{

	private:
	int			_fd;
	Host*		_host;
	Request*	_request;
	Response*	_response;

	public:

		Client();
		Client(int fd, Host *host, Request *req, Response *resp);
		~Client();
		Client(Client &Copy);
		Client	&operator=(Client &Rhs);

		int		getfd();
		Host	*getHost();

		bool	isReq(); //returns 1 if request has been fully received
		bool	isResp(); // returns 1 if response is ready to send

};


#endif
