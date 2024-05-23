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
		
		static int					getNextWord(std::istream& istream, std::string& word);
		static std::stringstream&	getNextBlock(std::istream& istream, std::stringstream& ostream);
		static inline void			skipSpace(std::istream& istream);
		static void					skipComment(std::istream& istream);
		static void					parseQuote(std::istream& istream, std::string& dest);
		static inline bool			checkSemiColon(std::istream& istream);

		static void				parseHostName(std::istream& istream, Host& host);
		static void				parseValues(std::istream& istream, std::deque<std::string>& words, int maxNbr);

		// static void				parseEscape(std::istream& istream);

		static void				parseHostBlock(std::stringstream& hostBlock, Host& host);
		static void				parseLocationBlock(std::istream& locationBlock, Location& location);
		static void				parseCGIConfigBlock(std::istream& cgiStream, CGIConfig& cgiConfig);

		static void				handleToken(std::stringstream& istream, const std::string& token, Host& host);
		static void				parseLocation(std::stringstream& istream, Host& host);
		static void				parseCGIConfig(std::stringstream& istream, Host& host);
		static void				parsePort(std::istream& istream, Host& host);
		static void				parseHost(std::istream& istream, Host& host);
		static void				parseServerName(std::istream& istream, Host& host);
		static void				parseBodyMaxSize(std::istream& istream, Host& host);
		
		static void				handleLocationToken(std::istream& istream, const std::string& token, Location& location);
		static void				parseAllowedMethods(std::istream& istream, bool (&dest)[METHODS_NBR]);
		static void				parseRedirection(std::istream& istream, Location& location);

		static void				handleCGIConfigToken(std::istream& istream,  const std::string& token, CGIConfig& location);

		static void				parsePath(std::istream& istream, std::string& dest);
		static void				parseBoolean(std::istream& istream, bool& dest);
		static void				parseUri(std::istream& istream, std::string& dest);

	public:

		static int				parseConfigFile(const char* path);

		class IParseConfigException : public std::exception {
			public:
				virtual const char*	what(void) const throw() = 0;
		};

		class FileException : public IParseConfigException {
			public:
				virtual const char*	what(void) const throw() = 0;
		};

		class InvalidFileTypeException : public FileException {
			public:
				virtual const char*	what(void) const throw() {
					return ("invalid config file type");
				}
		};

		class FileOpenException : public FileException {
			public:
				virtual const char*	what(void) const throw() {
					return ("could not open config file");
				}
		};

		class StreamException : public IParseConfigException {
			public:
				virtual const char*	what(void) const throw() {
					return ("error with the stream");
				}
		};

		class UnclosedQuoteException : public IParseConfigException {
			public:
				virtual const char*	what(void) const throw() {
					return ("unclosed quotes");
				}
		};

		class UnclosedBlockException : public IParseConfigException {
			public:
				virtual const char*	what(void) const throw() {
					return ("block has unclosed braces");
				}
		};

		class UnexpectedBraceException : public IParseConfigException {
			public:
				virtual const char* what(void) const throw() {
					return ("closing brace without prior opening brace");
				}
		};

		class LastBlockException : public IParseConfigException {
			public:
				virtual const char* what(void) const throw() {
					return ("last block reached");
				}
		};

		class MissingSemicolonException : public IParseConfigException {
			public:
				virtual const char*	what(void) const throw() {
					return ("missing semicolon (`;')");
				}
		};

		class MissingOpeningBraceException : public IParseConfigException {
			public:
				virtual const char*	what(void) const throw() {
					return ("missing opening brace `{'");
				}
		};

		class UnexpectedTokenException : public IParseConfigException {
			private:
				const std::string	message;
			public:
				UnexpectedTokenException(const std::string& param)
					: message("unexpected token (`" + param + "')") {}
				virtual ~UnexpectedTokenException(void) throw() {}
				virtual const char*	what(void) const throw() {return (message.c_str());}
		};

		class MissingTokenException : public IParseConfigException {
			private:
				const std::string	message;
			public:
				MissingTokenException(const std::string& param)
					: message("missing token or field (`" + param + "')") {}
				virtual ~MissingTokenException(void) throw() {}
				virtual const char*	what(void) const throw() {return (message.c_str());}
		};

		class UnknownTokenException : public IParseConfigException {
			private:
				const std::string	message;
			public:
				UnknownTokenException(const std::string& param)
					: message("unknown token (`" + param + "')") {}
				virtual ~UnknownTokenException(void) throw() {}
				virtual const char*	what(void) const throw() {return (message.c_str());}
		};

		class TooManyValuesException : public IParseConfigException {
			private:
				const std::string	message;
			public:
				TooManyValuesException(const std::string& param)
					: message("too many values for given token (`" + param + "')") {}
				virtual ~TooManyValuesException(void) throw() {}
				virtual const char*	what(void) const throw() {return (message.c_str());}
		};
		
};

#endif // ICONFIG_HPP
