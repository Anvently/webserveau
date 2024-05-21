#ifndef IPARSECONFIG_HPP
# define IPARSECONFIG_HPP

#include <Host.hpp>
#include <iostream>
#include <fstream>
#include <ILogger.hpp>
#include <sstream>
#include <string>
#include <deque>
#include <stdint.h>

//Parse config file into host vector

class IParseConfig
{
	private:

		virtual ~IParseConfig() = 0;

		static std::ifstream	_fileStream;
		static int				_lineNbr;
		static int				_lineNbrEndOfBlock;
		static std::string		_lineBuffer;

		static bool				checkFilePath(const char* path);
		static int				openFile(const char* path);
		

		// static std::string		getNextServerBlock(void);
		static int					getNextWord(std::istream& istream, std::string& word);
		static std::stringstream&	getNextBlock(std::istream& istream, std::stringstream& ostream);

		static void				parseHostName(std::istream& istream, Host& host);
		static void				parseValues(std::istream& istream, std::deque<std::string>& words, int maxNbr);

		static void				parseComment(std::istream& istream);
		static void				parseQuote(std::istream& istream, std::string& dest);
		// static void				parseEscape(std::istream& istream);

		static void				parseHostBlock(std::stringstream& hostBlock, Host& host);
		static void				parseLocationBlock(std::istream& locationBlock, t_location& location);

		static void				handleToken(std::stringstream& istream, const std::string& token, Host& host);
		static void				parseLocation(std::stringstream& istream, Host& host);
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

		class IParseConfigException : public std::exception {
			public:
				virtual const char*	what(void) const throw();
				virtual const char*	what(void) throw() = 0;
		};

		class FileException : public IParseConfigException {
			public:
				virtual const char*	what(void) throw() = 0;
		};

		class InvalidFileTypeException : public FileException {
			public:
				virtual const char*	what(void) throw();
		};

		class FileOpenException : public FileException {
			public:
				virtual const char*	what(void) throw();
		};

		class StreamException : public IParseConfigException {
			public:
				virtual const char*	what(void) throw();
		};

		class UnclosedQuoteException : public IParseConfigException {
			public:
				virtual const char*	what(void) throw();
		};

		class UnclosedBlockException : public IParseConfigException {
			public:
				virtual const char*	what(void) throw();
		};

		class UnexpectedBraceException : public IParseConfigException {
			public:
				virtual const char* what(void) throw();
		};

		class LastBlockException : public IParseConfigException {
			public:
				virtual const char* what(void) throw();
		};

		class MissingSemicolonException : public IParseConfigException {
			public:
				virtual const char*	what(void) throw();
		};

		class MissingOpeningBraceException : public IParseConfigException {
			public:
				virtual const char*	what(void) throw();
		};

		class UnexpectedTokenException : public IParseConfigException {
			private:
				const std::string	fieldName;
				std::string			message;
			public:
				UnexpectedTokenException(const std::string& param);
				virtual ~UnexpectedTokenException(void) throw();
				virtual const char*	what(void) const throw();
				virtual const char*	what(void) throw();
		};

		class MissingTokenException : public IParseConfigException {
			private:
				const std::string	fieldName;
				std::string			message;
			public:
				MissingTokenException(const std::string& param);
				virtual ~MissingTokenException(void) throw();
				virtual const char*	what(void) const throw();
				virtual const char*	what(void) throw();
		};

		class UnknownTokenException : public IParseConfigException {
			private:
				const std::string	fieldName;
				std::string			message;
			public:
				UnknownTokenException(const std::string& param);
				virtual ~UnknownTokenException(void) throw();
				virtual const char*	what(void) const throw();
				virtual const char*	what(void) throw();
		};

		class TooManyValuesException : public IParseConfigException {
			private:
				const std::string	fieldName;
				std::string			message;
			public:
				TooManyValuesException(const std::string& param);
				virtual ~TooManyValuesException(void) throw();
				virtual const char*	what(void) const throw();
				virtual const char*	what(void) throw();
		};
		
};

#endif // ICONFIG_HPP
