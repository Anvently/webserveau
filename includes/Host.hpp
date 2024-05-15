#ifndef HOST_HPP
# define HOST_HPP

#include <iostream>
#include <vector>
#include <list>
#include <map>
#include <string>
// #include <Client.hpp>

class IParseConfig;
class Client;

typedef struct location
{
	std::string	root;
	bool		methods[3]; //GET-POST-DELETE
	bool		dir_listing; //ignore is default_uri != 0
	std::string	default_uri;

	bool		upload; //ignore if POST not allowed
	std::string	dir_upload;

	bool	redirection;
	std::map<int, std::string>	addr_redir;

}	t_location;

typedef struct cgi_loc
{
	std::string	root;
	bool		methods[3];
	std::string	exec; //example '/bin/bash'
} cgi_loc;


class	Host {

	private:

		Host() {} friend class IParseConfig; //Throw an exception if block is invalid
		//Construct from what ?
		Host(Host &Copy);
		~Host();

		Host	&operator=(Host &rhs);
		
		// friend class IParseConfig		Host(const std::string& block); //Throw an exception if block is invalid

		// friend class	IParseConfig;

		static std::list<Host>			hostList;

		std::string						_host;
		int								_port;
		int								_client_max_size;
		std::string						_dir_errors;
		std::vector<std::string>		_server_names;
		std::map<std::string, location>	_locations;
		cgi_loc							_php_loc;
		cgi_loc							_py_loc;
		std::list<Client*>				_Clients; //<fd, Client> ?
		// a log file per Host ?

	public:

		static int						addHost(Host& host);

		std::string	getPort() const;
		int	getMaxSize() const;
		std::string	getHost() const;
		Client	*getClient(int fd);
		location	&getLocation(std::string const &path) const; //return the "/" loc if no match
		bool	checkServerName(std::string const &name) const; // check if the name refers to a server_name

		void	removeClient(int fd); //remove the Client associated with fd
		void	addClient(int fd, Client *newClient);

};


#endif
