#ifndef IPARSECONFIG_HPP
# define IPARSECONFIG_HPP

#include <Host.hpp>
#include <iostream>
#include <fstream>
#include <ILogger.hpp>
#include <sstream>
#include <string>

//Parse config file into host vector

class IParseConfig
{
	private:

		virtual ~IParseConfig() = 0;

		static std::ifstream	_fileStream;
		static int				_fileLineNum;
		static std::string		_lineBuffer;
		static int				_bufferLineNbr;

		static bool				checkFilePath(const char* path);
		static int				openFile(const char* path);
		
		static std::string		getNextServerBlock(void);

		static std::string		getNextBlock(std::istream& stream);

		static void				parseServerBlock(const std::string& block);
		static void				parseLocation(t_location* location, const std::string& block);


	public:

		static int				parseConfigFile(const char* path);

		class FileException : public std::exception {
			public:
				virtual const char*	what(void) const throw() = 0;
		};

		class InvalidFileTypeException : public FileException {
			public:
				virtual const char*	what(void) const throw();
		};

		class FileOpenException : public FileException {
			public:
				virtual const char*	what(void) const throw();
		};

		class FileStreamException : public std::exception {
			public:
				virtual const char*	what(void) const throw();
		};

		class UnclosedBlockException : public std::exception {
			public:
				virtual const char*	what(void) const throw();
		};

		class UnexpectedBraceException : public std::exception {
			public:
				virtual const char* what(void) const throw();
		};

		class LastBlockException : public std::exception {
			public:
				virtual const char* what(void) const throw();
		};
		
};

#endif // ICONFIG_HPP
