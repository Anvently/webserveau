#ifndef HOST_HPP
# define HOST_HPP

#include <iostream>
#include <vector>
#include <list>
#include <map>
#include <string>
#include <deque>
#include <Request.hpp>
#include <Client.hpp>
#include <Request.hpp>
// #include <Client.hpp>


class IParseConfig;
class Client;

struct Location
{
	Location(void);
	Location(const Location&);

	std::string					root;
	bool						methods[3]; //GET-POST-DELETE
	bool						dir_listing; //ignore is default_uri != 0
	std::string					default_uri;

	bool						upload; //ignore if POST not allowed
	std::string					upload_root;

	// bool	redirection;
	std::map<int, std::string>	addr_redir;

	bool						operator==(const Location&) const;
};

std::ostream&	operator<<(std::ostream& os, const Location& location);

struct CGIConfig
{
	CGIConfig(void);
	CGIConfig(const CGIConfig&);

	std::string					exec; //example '/bin/bash'
	std::string					root;
	bool						methods[METHOD_NBR]; //GET-POST-DELETE

	bool						operator==(const CGIConfig&) const;
};

std::ostream&	operator<<(std::ostream& os, const CGIConfig& CGIConfig);

class	Host {

	private:

		friend class IParseConfig;
		friend std::ostream&	operator<<(std::ostream& os, const Host& host);

		Host	&operator=(Host &rhs);

		// friend class IParseConfig		Host(const std::string& block); //Throw an exception if block is invalid

		// friend class	IParseConfig;

		static std::list<Host>				_hostList;
		static std::vector<Location>		_locations;
		static std::vector<CGIConfig>		_cgis;

		std::string							_addr;
		std::string							_port;
		int									_body_max_size;
		std::string							_dir_errors;
		std::vector<std::string>			_server_names;
		std::map<std::string, Location*>	_locationMap;
		std::map<std::string, CGIConfig*>	_cgiMap;
		std::list<Client*>					_clients; //<fd, Client> ?
		// a log file per Host ?

		void				addCGIConfig(const std::deque<std::string>& names, CGIConfig& cgiConfig);
		void				addLocation(const std::deque<std::string>& names, Location& location);

		void				printProperties(std::ostream& os) const;

		int					checkRedirection(Location& location, const Request& request) const;
		static inline int	assertRequestType(Location*, CGIConfig*, const Request&);
		int					checkDirRessource(Location& location, Request& request) const;
		int					checkLocationRules(Location& location, const Request& request) const;
		int					checkCGIRules(CGIConfig& cgi, const Request& request) const;
		static bool			checkRessourcePath(const std::string& path, int type = 0);

	public:

		Host(); //Throw an exception if block is invalid
		//Construct from what ?
		Host(const Host &Copy);
		~Host();

		static void							addHost(Host& host);
		static void							removeHost(Host* host);
		static std::list<Host>::iterator	findHost(Host* host);

		const std::string&				getPort() const;
		int								getMaxSize() const;
		const std::string&				getAddr() const;
		Client							*getClientByFd(int fd) const;
		Location*						getLocation(std::string const &path) const; //return the "/" loc if no match
		CGIConfig*						getCGIConfig(std::string const &path) const;
		const std::vector<std::string>&	getServerNames(void) const;
		std::list<Client*>::const_iterator	getClientListBegin(void) const;
		std::list<Client*>::const_iterator	getClientListEnd(void) const;
		bool							checkServerName(std::string const &name) const; // check if the name refers to a server_name

		void							removeClient(Client*); //remove the Client associated with fd
		void							addClient(Client *newClient);

		void							shutdown(void);

		void							printShort(std::ostream& os) const;
		void							printFull(std::ostream& os) const;

		int								checkRequest(Request& request) const;

};

std::ostream&	operator<<(std::ostream& os, const Host& host);

#endif
