#ifndef IPARSECONFIG_HPP
# define IPARSECONFIG_HPP

#include <Host.hpp>
#include <iostream>
#include <fstream>
#include <ILogger.hpp>
#include <sstream>
#include <string>
#include <deque>

//Parse config file into host vector

class IParseConfig
{
	private:

		virtual ~IParseConfig() = 0;

		static std::ifstream	_fileStream;
		static int				_lineNbr;
		static int				_lineNbrFile;
		static std::string		_lineBuffer;

		static bool				checkFilePath(const char* path);
		static int				openFile(const char* path);
		

		// static std::string		getNextServerBlock(void);

		static int				parseWord(std::istream& istream, std::string& word);
		static void				parseValues(std::istream& istream, std::deque<std::string>& words, int maxNbr = INT32_MAX);
		static void				parseQuote(std::istream& istream, std::string& dest);
		static void				parseComment(std::istream& istream);
		// static void				parseEscape(std::istream& istream);

		static void				parseHostName(std::istream& istream, Host& host);
		static std::stringstream&	getNextBlock(std::istream& istream, std::stringstream& ostream);

		static void				parseHostBlock(std::istream& hostBlock, Host& host);
		static void				parseLocationBlock(std::istream& locationBlock, t_location& location);

		static void				handleToken(std::istream& istream, const std::string& token, Host& host);
		static void				parseLocation(std::istream& istream, Host& host);
		static void				parsePort(std::istream& istream, Host& host);
		static void				parseHost(std::istream& istream, Host& host);
		static void				parseServerName(std::istream& istream, Host& host);
		static void				parseBodyMaxSize(std::istream& istream, Host& host);

		static void				handleLocationToken(std::istream& istream, const std::string& token, t_location& location);
		static void				parseAllowedMethods(std::istream& istream, t_location& location);
		static void				parseRedirection(std::istream& istream, t_location& location);

		static void				parsePath(std::istream& istream, std::string& dest);
		static void				parseBoolean(std::istream& istream, bool& dest);
		static void				parseUri(std::istream& istream, std::string& dest);

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

		class UnclosedQuoteException : public std::exception {
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

		class MissingSemicolonException : public std::exception {
			public:
				virtual const char*	what(void) const throw();
		};

		class UnexpectedTokenException : public std::exception {
			public:
				virtual const char*	what(void) const throw();
		};

		class UnknownTokenException : public std::exception {
			public:
				virtual const char*	what(void) const throw();
		};
		
};

#endif // ICONFIG_HPP
