#include <ILogger.hpp>

int				ILogger::_logLvlConsole = LOG_DFT_LVL_CONSOLE;
int				ILogger::_logLvlFile = LOG_DFT_LVL_LOGFILE;
std::ofstream	ILogger::_logFile;
std::time_t		ILogger::_startTime = time(NULL);

ILogger::ILogger(void) {}

/// @brief Printf equivalent without flag formats.
/// @param lvl Defines the log level of the text to be printed/written. A level greater
/// than configured lvl is ignored. ```0``` to ignore configured level.
/// @param format 
/// @param 
void	ILogger::log(int lvl, const char* format, ...)
{
	va_list	args;

	if (lvl > ILogger::_logLvlConsole && lvl > ILogger::_logLvlFile)
		return ;
	if (LOG_AUTO_LOGFILE && ILogger::_logFile.is_open() == false)
		ILogger::openLogFile(std::string(LOG_DFT_LOGFILE_PATH));
	// std::cout << "PING : " << (int)_logLvlConsole << " | " << (int)lvl << "\n";
	if (ILogger::_logLvlConsole >= lvl)
	{
		std::cout << TERM_CL_BOLD << colors[lvl];
		printTimestamp(std::cout) << TERM_CL_RESET;
	}
	if (ILogger::_logLvlFile >= lvl && ILogger::_logFile.good())
	{
		if (LOG_LOGFILE_COLOR)
		{
			ILogger::_logFile << TERM_CL_BOLD << colors[lvl];
			printTimestamp(ILogger::_logFile) << TERM_CL_RESET;
		}
		else
			printTimestamp(ILogger::_logFile);
	}
	va_start(args, format);
	parseFormat(format, &args, lvl);
	va_end(args);
}

void	ILogger::parseFormat(const char* format, va_list* args, int lvl)
{
	if (!format || !*format)
		return ;
	while (*format)
	{
		if (*format == '%')
		{
			switch (*(format + 1))
			{
				case '%':
					print('%', lvl);
					break;

				case 'd':
					print<int>(args, lvl);
					break;

				case 'i':
					print<int>(args, lvl);
					break;
				
				case 's':
					if (*(format + 2) == 's')
					{
						print<std::string*>(args, lvl);
						++format;
					}
					else
						print<char*>(args, lvl);
					break;

				case 'f':
					print<double>(args, lvl);
					break;

				case 'u':
					print<unsigned int>(args, lvl);
					break;

				case 'l':
					if (*(format + 2) == 'd')
						print<long>(args, lvl);
					else if (*(format + 2) == 'u')
						print<unsigned long>(args, lvl);
					else if (*(format + 2) == 'f')
						print<double>(args, lvl);
					else
						break;;
					++format;
					break;

				default:
					return ;
					break;
			}
			++format;
		}
		else
			print(*format, lvl);
		++format;
	}
}

/// @brief Open given log file file in append mode. If a log file was already opened,
/// it is closed before.
/// @param path 
/// @return ```0``` for success else ```1```.
int	ILogger::openLogFile(const std::string& path)
{
	return (ILogger::openLogFile(path.c_str()));
}

/// @brief Open given log file file in append mode. If a log file was already opened,
/// it is closed before.
/// @param path 
/// @return ```0``` for success else ```1```.
int	ILogger::openLogFile(const char* path)
{
	if (path == NULL)
		return (1);
	if (ILogger::_logFile.is_open())
		ILogger::_logFile.close();
	ILogger::_logFile.open(path, std::ofstream::app);
	if (_logFile.fail())
		return (1);
	// May want to print date;
	return (0);
}

void	ILogger::closeLogFile(void)
{
	ILogger::_logFile.close();
}

/// @brief Update log levels. ```LOG_NO_CHANGE``` can be used.
/// @param lvlConsole 
/// @param lvlFile 
void	ILogger::setLogLvl(int lvlConsole, int lvlFile)
{
	if (lvlConsole >= 0 && lvlConsole <= LOG_LVL_MAX)
		ILogger::_logLvlConsole = lvlConsole;
	else if (lvlConsole > LOG_LVL_MAX)
		ILogger::_logLvlConsole = LOG_LVL_MAX;
	if (lvlFile >= 0 && lvlFile <= LOG_LVL_MAX)
		ILogger::_logLvlFile = lvlFile;
	else if (lvlFile > LOG_LVL_MAX) 
		ILogger::_logLvlFile = LOG_LVL_MAX;
}

inline std::ostream&	ILogger::printTimestamp(std::ostream& os)
{
	return (os << '[' << std::time(NULL) - ILogger::_startTime << "] ");
}
