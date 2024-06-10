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

template <typename T>
  std::string NumberToString ( T Number )
  {
     std::ostringstream ss;
     ss << Number;
     return ss.str();
  }

static std::map<int, std::string> ResponseLine{{100, "Continue"}, {200, "OK"}, {201, "Created"}, {204, "No Content"}, \
{300, "Multiple Choices"}, {301, "Move Permanently"}, {302, "Found"}, {307, "Temporary Redirect"}, \
{308, "Permanent Redirect"}, {400, "Bad Request"}, {401, "Unauthorized"}, {403, "Forbidden"}, {404, "Not Found"}, \
{405, "Method Not Allowed"}, {408, "Request Timeout"}, {413, "Content Too Large"}, {414, "URI Too Long"}, \
{415, "Unsupported Media Type"}, {417, "Expectation Failed"}, {500, "Internal Server Error"}, {501, "Not Implemented"}, \
{505, "HTTP Version Not Supported"}};


#include<fstream>

std::string _line;
int _index = 0;

int getLine(std::string &buffer)
{
    size_t  idx = buffer.find("\r\n", 0);
    if (idx == std::string::npos)
        return (0);
    _line = buffer.substr(0, idx);
    _index += idx + 2;
    buffer = buffer.substr(idx + 2, std::string::npos);
    return (1);
}

int extract_header()
{
  std::cout << _line << std::endl;
  return (0);
}
#include <cstring>

int	main(int, char **, char **env)
{
	  int fd = open("./result", O_CREAT | O_RDWR | O_TRUNC);
    int infd = open("./inbody", O_RDONLY);
    //unlink("./inbody");
    dup2(fd, STDOUT_FILENO);
    dup2(infd, STDIN_FILENO);
    setenv("REQUEST_METHOD", "POST", 1);
    setenv("QUERY_STRING", "", 1);
    std::cout << getenv("QUERY_STRING") << std::endl;
    setenv("GATEWAY_INTERFACE", "CGI/1.1", 1); // which version to use ?
    setenv("SERVER_PROTOCOL", "HTTP/1.1", 1);
   // setenv("SERVER_NAME", request.getHeader("Host").c_str(), 1);
    //setenv("REMOTE_ADDR", client.getStrAddr().c_str(), 1);
    //setenv("SERVER_PORT", IntToString(client.getAddrPort(), 10).c_str(), 1);
    setenv("PATH_INFO", "./upl/", 1);
    setenv("PATH_TRANSLATED", "/home/lmahe/Documents/cercle_5/webserv/webserv/cgi_test/upl", 1); //TODODODODODODO
    setenv("SCRIPT_NAME", "/home/lmahe/Documents/cercle_5/webserv/webserv/cgi_test/upload.php", 1);
    setenv("CONTENT_TYPE", "multipart/form-data; boundary=---------------------------73", 1);
    setenv("CONTENT_LENGTH", "173", 1);
    char  *argv[3];
    argv[0] = strdup("/usr/bin/php");
    argv[1] = strdup("/home/lmahe/Documents/cercle_5/webserv/webserv/cgi_test/upload.php");
    argv[2] = NULL;
    execv(argv[0], argv);
    std::cout << "FAILED" << std::endl;


}
