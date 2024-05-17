#ifndef LISTENSERVER_HPP
#define LISTENSERVER_HPP

#include <list>
#include <iterator>
#include <algorithm>
#include <iostream>
#include <Host.hpp>
#include <Client.hpp>
#include <Header.hpp>
#include <ILogger.hpp>
#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <netinet/ip.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <string>
#include <string.h>
#include <IObject.hpp>


class ListenServer : public IObject
{
	private:

		ListenServer(void);
		ListenServer(std::string const &hostAddr, std::string const &hostPort);


		static	std::list<ListenServer>	serverList;

		int					_sockFd;
		std::list<Host*>	_hostList;
		std::string			_ip;
		std::string			_port;
		// ....

		static ListenServer	*addServer(const std::string& hostAddr, const std::string& hostPort);

	public:

		virtual ~ListenServer();

		static int	addHost(Host* host);

		static int	startServers(int epollfd);
		static int	closeServers(void);

		void		pushHost(Host* host);

		int			start(int epollfd); //open socket
		int			terminate(void);

		bool			isMatch(std::string const &hostAddr, std::string const &hostPort);

		/// @brief Check for
		/// @param header
		/// @return
		Host*	bindClient(Header& header);



};


#endif
