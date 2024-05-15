#include <iostream>
#include <ILogger.hpp>

int	main(void)
{
	ILogger::setLogLvl(LOG_LVL_MAX, LOG_LVL_MAX);

		LOGE("THis is a test : %d", 45);
		LOGI("This is an info %d", 45);
		LOGW("this is a warning %s", "");
		LOGD("this is debug");
		LOGV("this is verbose");
	return (0);
}