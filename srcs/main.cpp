#include <iostream>
#include <ILogger.hpp>
#include <IParseConfig.hpp>

int	main(void)
{
	ILogger::setLogLvl(LOG_LVL_MAX, LOG_LVL_MAX);

	IParseConfig::parseConfigFile("conf/template.conf");
	return (0);
}