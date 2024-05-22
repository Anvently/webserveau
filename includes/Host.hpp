#ifndef HOST_HPP
# define HOST_HPP

#include <iostream>
#include <vector>
#include <list>
#include <map>
#include <string>
// #include <Client.hpp>

#define METHODS_NBR 3

class IParseConfig;
class Client;

struct Location
{
	std::string	root;
	bool		methods[3]; //GET-POST-DELETE
	bool		dir_listing; //ignore is default_uri != 0
	std::string	default_uri;

	bool		upload; //ignore if POST not allowed
	std::string	upload_root;

	bool	redirection;
	std::map<int, std::string>	addr_redir;
};

struct CGIConfig
{
	std::string	exec; //example '/bin/bash'
	std::string	root;
	bool		methods[METHODS_NBR]; //GET-POST-DELETE
};


class	Host {

	private:

		friend class IParseConfig;

		Host	&operator=(Host &rhs);
		
		// friend class IParseConfig		Host(const std::string& block); //Throw an exception if block is invalid

		// friend class	IParseConfig;

		static std::list<Host>			_hostList;

		std::string						_name;
		int								_port;
		int								_client_max_size;
		std::string						_dir_errors;
		std::vector<std::string>		_server_names;
		std::map<std::string, Location>	_locations;
		CGIConfig						_php_loc;
		CGIConfig						_py_loc;
		std::list<Client*>				_clients; //<fd, Client> ?
		// a log file per Host ?

	public:

		Host(); //Throw an exception if block is invalid
		//Construct from what ?
		Host(const Host &Copy);
		~Host();

		static void						addHost(Host& host);

		int	getPort() const;
		int	getMaxSize() const;
		std::string	getHost() const;
		Client	*getClientByFd(int fd) const;
		Location*	getLocation(std::string const &path); //return the "/" loc if no match
		bool	checkServerName(std::string const &name) const; // check if the name refers to a server_name

		void	removeClient(Client*); //remove the Client associated with fd
		void	addClient(Client *newClient);

};


#endif
