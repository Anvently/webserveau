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
#include <dirent.h>
#include <sys/stat.h>


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


	std::string	_body;


	std::string	dir = "dir";
	DIR	*d = opendir(dir.c_str());
	if (!d)
		return 0; //overload the verbose generate to make a 500 error response
	struct dirent	*files;
	_body += "<html><head><title> dir list </title></head><body><style>";
	_body += "#two{text-align:center;font-size:250%;color:lightblue}</style>";
	_body += "<p id=\"two\"> Contents of " + dir +  " </p>";
	_body += "<ul>";
	while ((files = readdir(d)))
	{
		if (files->d_type != 8 && files->d_type != 4)
			continue;
		_body += "<li><a href = \"";
		_body += files->d_name;
		_body += "\">";
		_body += files->d_name;
		_body += "</a></li>";
	}
	_body += "</ul></body></html>";
	std::cout << _body;
	return (0);
}

//<meta http-equiv=\"refresh\"content=\"0; url=" + redir + "\">
