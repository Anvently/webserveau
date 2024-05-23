#include <Header.hpp>

int	main()
{
	Header	hd;

	std::string	test = "POST /test HTTP/1.1\r\n";
				test += "Host: foo.example\r\n";
				test += "Content-Type: application/x-www-form-urlencoded\r\n";
				test +=	"Content-Length: 27\r\n\r\n";
				test +=	"field1=value1&field2=value2";

	hd.parseInput(test);
	hd.formatHeaders();
	hd.printHeaders();
	hd.printRequest();
	std::cout << std::endl << "The body is: " << test;
}
