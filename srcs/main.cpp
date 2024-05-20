#include <iostream>
#include <ILogger.hpp>
#include <stdint.h>
#include <IParseConfig.hpp>

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
	initLogs();

	// void*	pigeonPtr = new Pigeon();
	// void*	zebrePtr = new Zebra();

	// Zebra*	pigeonZebre = dynamic_cast<Zebra*>((Pigeon *)pigeonPtr);

	// if (pigeonZebre == NULL)
	// 	LOGE("FAILED");
	// else
	// 	LOGI("SUCCESS : %s", "");
	// (void)zebrePtr;
	// LOGI("size of uinptr = %d", sizeof(Zebra*));
	// std::string buffer = "Hi this is! a test\n \nwdw\n\n";
	// std::stringstream	stream(buffer);
	// stream << '\0' << "defde ";
	// while (stream.eof() == false)
	// {
		// std::string word;
		// stream >> word;
		// std::cout << word << '|';
	// }
	IParseConfig::parseConfigFile("conf/template.conf");
	ILogger::clearFiles();
	return (0);
}