#include <iostream>
#include <ILogger.hpp>
#include <stdint.h>
#include <sys/epoll.h>
#include <ListenServer.hpp>
#include <IControl.hpp>
#include <IParseConfig.hpp>

#define EPOLL_EVENT_MAX_SIZE 100

// class Pigeon
// {
// 	private:
// 		int age;
// 		int size;
// 	public:
// 		Pigeon();
// 		// virtual	~Pigeon();
// 		// virtual void	function(void);
// };

// class Zebra
// {
// 	private:
// 		int  age;
// 		unsigned long color;
// 	public:
// 		Zebra();
// 		// virtual	~Zebra();
// 		// virtual void	function(void);
// };

// Pigeon::Pigeon() {
// 	(void)age;
// 	(void)size;
// }

// // Pigeon::~Pigeon() {}
// // Zebra::~Zebra() {}

// Zebra::Zebra() {
// 	(void)age;
// 	(void)color;
// }


static void	initLogs(void)
{
	ILogger::addStream(std::cout, LOG_CONFIG_DEBUG | LOG_COLORIZE_MSK);
	ILogger::addLogFile("logs/sessions.log", LOG_CONFIG_DEBUG);
	ILogger::addLogFile("logs/error.log", LOG_ERROR_MSK);
	ILogger::logDate(-1);
	ILogger::setInit();
	ILogger::printLogConfig();
}

int	main(void)
{
	int	epollfd = 0, nbr_events;
	struct epoll_event	events[EPOLL_EVENT_MAX_SIZE];

	initLogs();
	epollfd = epoll_create(1);
	if (epollfd < 0) {
		LOGE("Fatal error : could not create epoll");
		return (1);
	}
	IParseConfig::parseConfigFile("conf/template.conf");
	if (ListenServer::getNbrServer() == 0)
		return (0);
	ListenServer::startServers(epollfd);
	while (1) {
		nbr_events = epoll_wait(epollfd, events, EPOLL_EVENT_MAX_SIZE, 50);
		if (nbr_events) {
			if (IControl::handleEpoll(events, nbr_events) < 0)
				break;
		}
	}
	ListenServer::deleteServers();
	ILogger::clearFiles();
	return (0);
}