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

int	main()
{
	std::string output = "Content-Type: text/html\r\nStatus: 302\r\nLocation: http://ailleus.com:80/pasla/labas\r\n\r\nBody of size 15";
  int fd = open("../cgi_output", O_TRUNC | O_WRONLY);
  write(fd, output.c_str(), output.size());
  close(fd);
  char            *c_buffer = new char[124];
  std::fstream  fstr("../cgi_output");
  fstr.read(c_buffer, 124);
  std::string     buffer(c_buffer, fstr.gcount());
  fstr.close();
  while(getLine(buffer))
    {
        if (_line.empty())
            break;
        if (extract_header())
        {
            return (0);
        }
    }
    std::cout << "\nBuffer is at then end: " << buffer << " of size: " << buffer.size();
    struct stat stats;
    stat("../cgi_output", &stats);
    std::cout << "\nstats of file:" <<  stats.st_size;
    std::cout << "\nindex is: " << _index;

    fstr.open("../cgi_output");
    fstr.seekg(85);
    std::string out;
    fstr >> out;
    std::cout << out;

}
