#include <iostream>
#include <ILogger.hpp>
#include <IParseConfig.hpp>

static void	initLogs(void)
{
	ILogger::addStream(std::cout, LOG_CONFIG_DEBUG | LOG_COLORIZE_MSK);
	ILogger::addLogFile("logs/sessions.log", LOG_CONFIG_INFO);
	ILogger::addLogFile("logs/error.log", LOG_ERROR_MSK);
	ILogger::logDate(-1);
	ILogger::setInit();
	ILogger::printLogConfig();
}

int	main(void)
{
	initLogs();

	IParseConfig::parseConfigFile("conf/template.conf");
	ILogger::clearFiles();
	return (0);
}