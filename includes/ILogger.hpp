#ifndef ILOGGER_HPP
# define ILOGGER_HPP

#include <iostream>
#include <string>
#include <fstream>
#include <stdarg.h>
#include <ctime>

#define TERM_CL_RED "\033[31m"
#define TERM_CL_GREEN "\033[32m"
#define TERM_CL_YELLOW "\033[33m"
#define TERM_CL_BLUE "\033[34m"
#define TERM_CL_MAGENTA "\033[35m"
#define TERM_CL_CYAN "\033[36m"
#define TERM_CL_WHITE "\033[37m"
#define TERM_CL_RESET "\033[0m"
#define TERM_CL_BK_GREEN "\033[42m"
#define TERM_CL_BK_RED "\033[41m"
#define TERM_CL_BOLD "\033[1m"

/// @brief Provide an interface for logging messages in console and a log file
/// provided a logging level.
class ILogger
{
	private:

		ILogger(void);
		~ILogger(void);

		static int					_logLvlConsole;
		static int					_logLvlFile;

		static std::ofstream		_logFile;

		static std::time_t			_startTime;

		static inline std::ostream&	printTimestamp(std::ostream& os);

		static void					parseFormat(const char* format, va_list* args, int lvl);

		template <typename T>
		static inline void			print(T param, int lvl);

		template <typename T>
		static inline void			print(va_list* args, int lvl);

	public:

		enum LOG_LVL
		{
			LOG_NO_CHANGE = -1,
			LOG_DISABLE,
			LOG_ERROR,
			LOG_WARNING,
			LOG_INFO,
			LOG_DEBUG,
			LOG_VERBOSE
		};

		static void	log(int lvl, const char *format, ...);

		static int	openLogFile(const char* path);
		static int	openLogFile(const std::string& path);

		static void	setLogLvl(int lvlConsole, int lvlFile);

		static void	closeLogFile(void);
};


#define LOG_LVL_MAX (ILogger::LOG_VERBOSE)

#define LOGE(format, ...) ILogger::log(ILogger::LOG_ERROR, format"\n" __VA_OPT__(,) __VA_ARGS__);
#define LOGW(format, ...) ILogger::log(ILogger::LOG_WARNING, format"\n" __VA_OPT__(,) __VA_ARGS__);
#define LOGI(format, ...) ILogger::log(ILogger::LOG_INFO, format"\n" __VA_OPT__(,) __VA_ARGS__);
#define LOGD(format, ...) ILogger::log(ILogger::LOG_DEBUG, format"\n" __VA_OPT__(,) __VA_ARGS__);
#define LOGV(format, ...) ILogger::log(ILogger::LOG_VERBOSE, format"\n" __VA_OPT__(,) __VA_ARGS__);

#define LOG_AUTO_LOGFILE 1 //A logfile will be generated without calling openLogFile()
#define LOG_DFT_LOGFILE_PATH "webserv.log"
#define LOG_LOGFILE_COLOR 0 //Toggle colors inside log file

#define LOG_DFT_LVL_CONSOLE LOG_DISABLE
#define LOG_DFT_LVL_LOGFILE LOG_DISABLE

#define LOG_LVL_NO_CHANGE ILogger::LOG_NO_CHANGE

static const char*	colors[LOG_LVL_MAX + 1] = {
						TERM_CL_WHITE, //0 - Ignore level
						TERM_CL_RED, //Error
						TERM_CL_YELLOW, //Warning
						TERM_CL_GREEN, //Info
						TERM_CL_MAGENTA, //Debug
						TERM_CL_WHITE //Verbose
};

template <typename T>
inline void	ILogger::print(va_list* args, int lvl)
{
	T	var = va_arg(*args, T);
	print(var, lvl);
}

template <typename T>
inline void	ILogger::print(T param, int lvl)
{
	if (ILogger::_logLvlConsole >= lvl)
		std::cout << param;
	if (ILogger::_logLvlFile >= lvl && ILogger::_logFile.is_open())
		ILogger::_logFile << param;
}

#endif
