#include <string>
#include <iostream>
#include <vector>
#include <ctime>
#include <unistd.h>
#include<sys/time.h>
// int	pruneScheme(std::string &uri)
// {
// 	size_t	n = uri.find("://", 0);
// 	if (n != std::string::npos)
// 	{
// 		if (uri.substr(0, n + 3) != "http://")
// 			return (1);
// 		uri.erase(0, n + 3);
// 	}
// 	return (0);
// }

// int	pruneDelim(std::string &uri, std::string delim)
// {
// 	size_t	idx = uri.find(delim, 0);
// 	if (idx != std::string::npos)
// 		uri.erase(0, idx + delim.size());
// 	return (0);
// }

// std::string	extractPath(std::string &uri)
// {
// 	size_t	idx = uri.find("?", 0);
// 	if (idx != std::string::npos)
// 	{
// 		std::string	path = uri.substr(0, idx);
// 		uri.erase(0, idx + 1);
// 		return (path);
// 	}
// 	else
// 		return (uri);
// }

// int	checkPath(std::string &path)
// {
// 	int	level = 0;
// 	size_t	idx = 0;
// 	size_t	idxx = 0;
// 	std::string	subpath;
// 	std::vector<std::string> v;

// 	while ((idx = path.find("//", 0)) != std::string::npos)
// 		path.erase(idx, 1);
// 	while ((idx = path.find("/./", 0)) != std::string::npos)
// 		path.erase(idx, 2);
// 	idx = 0;
// 	while((idx = path.find("/", 0)) != std::string::npos)
// 	{
// 		subpath = path.substr(0, idx);
// 		path.erase(0, idx + 1);
// 		if (subpath == ".." && v.size() == 0)
// 			return (1);
// 		else if (subpath == ".." && v.size())
// 			v.pop_back();
// 		else
// 			v.push_back(subpath);
// 		subpath.clear();
// 	}
// 	v.push_back(path);
// 	path.clear();
// 	for (std::vector<std::string>::iterator it = v.begin(); it != v.end(); it++)
// 		path+= "/" + *it;
// 	path.erase(0,1);
// 	return (0);
// }

// std::string	getFilename(std::string path)
// {
// 	std::string	root;
// 	if (path.empty() || path.back() == '/')
// 	{
// 		root = path;
// 		std::cout << "ROOT is = " << root << std::endl;
// 		return ("./");
// 	}
// 	size_t	idx = path.find_last_of("/");
// 	root = path.substr(0, idx +1);
// 	std::cout << "ROOT is = " << root << std::endl;
// 	return (path.substr(idx + 1));
// }

// void	prunePath(std::string &path)
// {
// 	std::string	filename = getFilename(path);
// 	std::cout << "\n filename = " << filename << "\n\n";
// 	std::string extension;
// 	size_t	idx = filename.find_last_of(".");
// 	if (idx == std::string::npos)
// 		extension = "";
// 	else
// 		extension = filename.substr(idx);
// 	std::cout << "EXTENSION = " << extension << std::endl;
// }


// int	main()
// {
// 	std::string uri = "/dir1/index.php?var1=%25s&var2=%A3we";
// 	std::string	path;
// 	pruneScheme(uri);
// 	std::cout << uri << std::endl << "\n";
// 	pruneDelim(uri, "@");
// 	std::cout << uri << std::endl << "\n";
// 	pruneDelim(uri, "/");
// 	path = extractPath(uri);
// 	std::cout << "\n path = " << path << std::endl << "\n";
// 	std::cout << "\n query = " << uri << "\n\n";
// 	std::cout << checkPath(path) << "\n";
// 	std::cout << path << std::endl << "\n";
// 	prunePath(path);
// }

int	main()
{
	struct timeval	first;
	struct timeval	sec;

	long long	x;
	long long	y;

	gettimeofday(&first, NULL);
	usleep(5000);
	gettimeofday(&sec, NULL);

	x = first.tv_sec * 1000 + first.tv_usec / 1000;
	y = sec.tv_sec * 1000 + sec.tv_usec / 1000;

	std::cout << y - x << "\n";

}
