#include <Request.hpp>
#include <string>
#include <iostream>

int	main()
{

	Request	req;
	std::string	test = "POST /test HTTP/1.1\r\n";
				test += "Host: foo.example\r\n";
				test += "Content-Type: text/plain\r\n";
				test += "Transfer-Encoding: chunked\r\n";
				test +=	"Content-Length: 27\r\n\r\n";
				test +=	"8\r\n";
				test += "Mozilla\n\r\n";
				test +=	"11\r\n";
				test += "Developer Network\r\n";
				test += "0\r\n";
				test += "\r\n";
				test += "GET / HTTP1/1\r\n";

req.parseHeaders(test);
std::cout << test << std::endl;
req.parseBody(test);
std::cout << test << std::endl;

std::cout << "-------------" << std::endl;
req.printHeaders();
}
