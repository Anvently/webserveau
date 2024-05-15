#ifndef LISTENSERVER_HPP
#define LISTENSERVER_HPP

#include <list>
#include <iostream>
#include <Host.hpp>
#include <Client.hpp>
#include <Header.hpp>

//static map 
//return host given a header

class ListenServer
{
	private:

		ListenServer(void);
		~ListenServer();

		static	std::list<ListenServer>	serverList;

		std::list<Host*>	_hostList;
		int					_sockFd;
		// ....

		static int	addServer(const std::string& hostAddr, const std::string& hostPort);

	public:

		static int	addHost(Host* host);
	
		static int	startServers(void);
		static int	closeServers(void);

		int			addHost(Host* host);

		int			start(void); //open socket
		int			close(void);

		/// @brief Check for 
		/// @param header 
		/// @return 
		Host*	bindClient(Header& header);


		
};


#endif 