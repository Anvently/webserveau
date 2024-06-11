#include <string>
#include <iostream>
#include <vector>
#include <ctime>
#include <unistd.h>
#include<sys/time.h>
#include<queue>
#include<iterator>
#include <map>
#include <fcntl.h>
#include <sys/stat.h>
#include <new>
#include <sstream>
#include <fstream>
#include <sys/types.h>
#include <unistd.h>
#include <cstring>
#include <algorithm>
#include <string>


// void	DynamicResponse::generateSimpleRedirBody()
// {
// 	std::string	line;
// 	line = itostr(hints.status) + " " + ResponseLine[hints.status];
// 	_body += "<html><head><title> " + line + " </title><meta http-equiv=\"refresh\"content=\"0; url=" + hints.redirList->front() + "\"></head><body><style>";
// 	_body += "#one{color:darkred;text-align:center;font-size:300%;}";
// 	_body += "#two{color:brown;text-align:center;font-size:150%;}</style>";
// 	_body += "<p id=\"one\">" + line + "</p>";
// 	_body += "<p id=\"two\">" + " The document can be found at" + "</p></body></html>";


// }

int	main(){

	std::string line = "300 Multiple Choices";
	std::string	_body;
	std::vector<std::string>	vec;
	vec.push_back("http://ici");
	vec.push_back("http://labas");
	vec.push_back("http://plutotla");

	_body += "<html><head><title> " + line + " </title></head><body><style>";
	_body += "#one{color:darkred;text-align:center;font-size:300%;}";
	_body += "#two{text-align:center;font-size:150%;}</style>";
	_body += "<p id=\"one\">" + line + "</p>";
	_body += "<p id=\"two\"> The document has been moved to: </p>";
	_body += "<ul>";
	for (std::vector<std::string>::const_iterator it = vec.begin(); it != vec.end(); it++)
	{
		_body += "<li><a href=\"" + *it + "\">" + *it + "</a></li>";
	}
	_body += "</ul>";
	_body += "</body></html>";

	int	fd = open("test.html", O_CREAT | O_TRUNC | O_WRONLY, 0777);
	write(fd, _body.c_str(), _body.size());

}

//<meta http-equiv=\"refresh\"content=\"0; url=" + redir + "\">
