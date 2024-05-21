#include <IParseConfig.hpp>
#include <sys/stat.h>

// #define WORD_IS_LAST 1
// #define WORD_IS_COMMENT 2
// #define WORD_IS_CITATION 3

#define WORD_IS_EOF (1 << 4)
#define WORD_IS_BRACED (1 << 3)
#define WORD_IS_CITATION (1 << 2)
#define WORD_IS_LAST (1 << 1)
#define WORD_IS_COMMENT (1 << 0)

std::ifstream	IParseConfig::_fileStream;
std::string		IParseConfig::_lineBuffer;
int				IParseConfig::_lineNbr = 1;
int				IParseConfig::_lineNbrEndOfBlock = 1;

bool	IParseConfig::checkFilePath(const char* path)
{
	struct stat	f_stat;

	if (stat(path, &f_stat) || S_ISREG(f_stat.st_mode) == false)
		return (false);
	return (true);
}

/// @brief Return a string containing the next block found in given istream.
/// Block  start at the first ```{``` encountered.
/// Block is defined by content delimited with ```{}```. Nested block are ignored.
/// ```\\``` is an escape character, ```{}``` within quote (```"```) are ignored.
/// Syntax error will raise exceptions. When the function reach eof without finding
/// any block ```LastBlockException``` is raised.
/// @param stream can be a ifstream or istringstream
/// @return 
std::stringstream&	IParseConfig::getNextBlock(std::istream& istream, std::stringstream& ostream)
{
	int						enclosedDepth = 0;
	bool					isEscaped = false, isQuoted = false;
	std::string::iterator	it, start, end;

	do
	{
		std::getline(istream, _lineBuffer);
		_lineNbrEndOfBlock++;
		start = _lineBuffer.begin();
		end = _lineBuffer.end();
		// LOGD("lineBuffer = %ss", &_lineBuffer);
		for (it = start; it != end; it++)
		{
			if (isEscaped)
				isEscaped = false;
			else if (*it == '\\')
				isEscaped = true;
			else if (*it == '"')
				isQuoted = !isQuoted;
			else if (isQuoted == false && *it == '{' && enclosedDepth++ == 0)
				start = it + 1;
			else if (isQuoted == false && *it == '}' && --enclosedDepth == 0)
			{
				end = it;
				break;
			}
			else if (isQuoted == false && *it == '}' && enclosedDepth < 0)
				throw (UnexpectedBraceException());
		}
		if (it != _lineBuffer.end()) //&& std::distance(it, _lineBuffer.end()) > 1)
		{
			if (istream.eof())
			{
				istream.clear();
				istream.seekg(istream.tellg() - std::distance(it, _lineBuffer.end() - 1));
			}
			else
				istream.seekg(istream.tellg() - std::distance(it, _lineBuffer.end()));
			_lineNbrEndOfBlock--;
		}
		// if (it != _lineBuffer.end());
		if (enclosedDepth != 0 || (enclosedDepth == 0 && (start != _lineBuffer.begin() || end != _lineBuffer.end())))
		{
			ostream << _lineBuffer.substr(std::distance(_lineBuffer.begin(), start), std::distance(start, end));
			if (enclosedDepth)
				ostream << '\n';
		}
	} while (istream.good() && istream.eof() == false && it == _lineBuffer.end());
	if (istream.bad() && istream.eof() == false)
		throw (StreamException());
	if (enclosedDepth > 0)
		throw (UnclosedBlockException());
	if (istream.eof() && end == _lineBuffer.end())
		throw (LastBlockException());
	return (ostream);
}

// /// @brief Return a string containing the next block found in config file.
// /// Block  start at the first ```{``` encountered.
// /// Block is defined by content delimited with ```{}```. Nested block are ignored.
// /// ```\\``` is an escape character, ```{}``` within quote (```"```) are ignored.
// /// Syntax error will raise exceptions. When the function reach eof without finding
// /// any block ```LastBlockException``` is raised.
// /// @param  
// /// @return 
// std::string		IParseConfig::getNextServerBlock(void)
// {
// 	if (_fileStream.is_open() == false || _fileStream.good() == false)
// 		throw(StreamException());
// 	return (getNextBlock(_fileStream));
// }

/// @brief Suppose that an opening quote has been detected in dest,
/// stream will be parsed storing every cited word in dest.
/// Unclosed quote will throw exception.
/// @param istream 
/// @param dest 
void	IParseConfig::parseQuote(std::istream& istream, std::string& dest)
{
	bool	isEscaped = false;
	char	c;

	while (istream.good())
	{
		c = istream.get();
		if (c == '\n')
			_lineNbr++;
		else if (c == '\\' && isEscaped == false)
		{
			isEscaped = true;
			continue;
		}
		else if (c == '"' && isEscaped == false)
			return;
		dest += c;
		isEscaped = false;
	}
	throw (UnclosedQuoteException());
}

/// @brief Suppose a comment token has already been found in the last read of the stream
/// Advance the stream to the next ```\\n``` or ```\\0```. Throw exception is not found.
/// @param istream 
void IParseConfig::parseComment(std::istream& istream)
{
	char	c;

	while (istream.good())
	{
		c = istream.get();
		if (c == '\n' || c == '\0')
		{
			_lineNbr++;
			return;
		}
	}
	// throw (StreamException());
}

/// @brief Read the next word in the stream and assign it to ```word``` string,
/// checking if it's a comment, if there are quote to expand, and if the word ends
/// with a semicolon.
/// If word is (or start with) a comment token, stream is readed until ```\\n```
/// or ```\\0```, and ```WORD_IS_COMMENT``` is returned. If word is a quoted sequence, the
/// full sequence is appended to word. If word ends with a semicolon (```;```).
/// then ```WORD_IS_LAST``` is returned. If word ends with a brace ```{}```, stream cursor
/// is moved back of one char and ```WORD_IS_LAST | WORD_IS_BRACED``` is returned.
/// ```WORD_IS_LAST``` and ```WORD_IS_CITATION``` can be ORed. ```WORD_IS_EOF``` means
/// the full stream was read.
/// @param istream 
/// @param word 
/// @return  ```-1``` if an erroor occured
int	IParseConfig::	getNextWord(std::istream& istream, std::string& word)
{
	char	c;
	bool	isEscaped = false;
	int		ret = 0;

	if (istream.eof() == true)
		return (WORD_IS_EOF);
	else if (istream.good() == false)
		return (-1);
	do {
		c = istream.get();
		if (c == '\n')
			_lineNbr++;
	} while (std::isspace(c) && istream.good());
	while (istream.good() && ((std::isspace(c) == false && c != ';' && c != '{' && c != '}') || isEscaped == true))
	{
		if (isEscaped == true)
		{
			word += c;
			isEscaped = false;
		}
		else if (c == '\\')
			isEscaped = true;
		else if (c == '"')
		{
			parseQuote(istream, word);
			ret |= WORD_IS_CITATION;
		}
		else if ((c == '/' && (word.size() == 1 && *word.begin() == '/')) || c == '#')
		{
			parseComment(istream);
			word.clear();
			return (WORD_IS_COMMENT);
		}
		else
			word += c;
		c = istream.get();
	}
	if (c == ';')
		ret |= WORD_IS_LAST;
	else if (c == '{' || c== '}')
	{
		istream.clear();
		istream.seekg(-1, std::ios_base::cur);
		ret |= WORD_IS_BRACED | WORD_IS_LAST;
	}
	else if (c == '\n')
		_lineNbr++;
	else if (istream.good() == false)
		ret |= WORD_IS_EOF;
	return (ret);
}

/// @brief Parse ```maxNbr``` words in ```istream``` until a semicolon (```;```) is found at the end of  
/// @param istream 
/// @param words 
/// @param maxNbr 
void	IParseConfig::parseValues(std::istream& istream, std::deque<std::string>& words, int maxNbr = INT32_MAX)
{
	std::string	word;
	int			i = 0, ret = 0;

	do
	{
		ret = getNextWord(istream, word);
		if ((ret & WORD_IS_COMMENT) == 0 && word != ";") //If word is not a comment
			words.push_front(word);
		if (ret & WORD_IS_LAST)
			break;
		i++;
		word.clear();
	} while (istream.good() && i < maxNbr);
	if (words.size() == 0 || (ret & WORD_IS_LAST) == 0)
		throw (MissingSemicolonException());
}

void	IParseConfig::parseHostBlock(std::stringstream& hostBlock, Host& host)
{
	std::string				token;
	int						ret;
	
	try
	{
		while (hostBlock.good())
		{
			token.clear();
			ret = getNextWord(hostBlock, token);
			if (ret == WORD_IS_EOF)
				break;
			if (ret == 0)
				handleToken(hostBlock, token, host);
			else if ((ret & WORD_IS_COMMENT) == 0)
				throw (UnexpectedTokenException(token));
		}
	}
	catch (IParseConfigException& e)
	{
		std::string	tmp;
		if (hostBlock.good())
			tmp = hostBlock.str().substr(hostBlock.tellg());
		else
		{
			tmp = "[End of block]";
		}
		LOGE("parsing host (%ss) configuration at line %d : %s\n -> ...%sl...", &host._name, _lineNbr, e.what(), &tmp);
		_lineNbr = _lineNbrEndOfBlock;
	}
}


void	IParseConfig::parseLocation(std::stringstream& istream, Host& host)
{
	std::deque<std::string>	locationsName;
	t_location				location;

	parseValues(istream, locationsName); //May need to check what happens gere
	std::stringstream	locationStream;
	_lineNbrEndOfBlock = _lineNbr;
	getNextBlock(istream, locationStream);
	std::string tmp = locationStream.str();
	// LOGD("location block start at = %d | _lineEoB = %d", _lineNbr, _lineNbrEndOfBlock);
	try {
		parseLocationBlock(locationStream, location);
	}
	catch (const LastBlockException& e) {
		throw (MissingOpeningBraceException());
	}
	catch (IParseConfigException& e)
	{
		std::string	tmp;
		if (locationStream.good())
			tmp = locationStream.str().substr(locationStream.tellg());
		else
		{
			tmp = "[End of block]";
		}
		LOGE("parsing host (%ss) location at line %d : %s\n -> %sl", &host._name, _lineNbr, e.what(), &tmp);
		_lineNbr = _lineNbrEndOfBlock;
	}
}

void	IParseConfig::parseLocationBlock(std::istream& locationBlock, t_location& location)
{
	std::string				token;
	int						ret;
	
	while (locationBlock.good())
	{
		token.clear();
		ret = getNextWord(locationBlock, token);
		if (ret == WORD_IS_EOF)
			break;
		if (ret == 0)
			handleLocationToken(locationBlock, token, location);
		else if ((ret & WORD_IS_COMMENT) == 0)
			throw (UnexpectedTokenException(token));
	}
}

int	IParseConfig::openFile(const char* path)
{
	try
	{
		if (!path || checkFilePath(path) == false)
			throw(InvalidFileTypeException());
		_fileStream.open(path, std::ifstream::in);
		if (_fileStream.fail() == true)
			throw(FileOpenException());
	}
	catch (FileException& e)
	{
		LOGE("%s : %s", path, e.what());
		return (1);
	}
	return (0);
}

void	IParseConfig::parseHostName(std::istream& istream, Host& host)
{
	std::string	word;
	int			ret = 0;

	while (istream.good())
	{
		word.clear();
		ret = getNextWord(istream, word);
		if (ret == WORD_IS_COMMENT)
			continue;
		else if (ret & WORD_IS_CITATION)
			throw (UnexpectedTokenException(word));
		else if (word.size() == 0)
			throw (MissingTokenException("host_name"));
		host._name = word;
		ret = getNextWord(istream, word);
		if (!(ret & WORD_IS_BRACED))
			throw (TooManyValuesException("host name"));
		break;
	}
}

/// @brief Open given file and try to parse server block, creating server/host 
/// objects on the go. Object are created but server is not start yet.
/// Exception generated by syntax error are catched and error message are printed.
/// Any syntax error outside host block will stop the parsing. Any syntax
/// error within an host or location block will stop corresponding block parsing.
/// @param path 
/// @return ```0``` if no parsing error occured. ```1``` is at least 1 parsing
/// error occured.
int	IParseConfig::parseConfigFile(const char* path)
{
	if (openFile(path))
		return (1);
	while (1)
	{
		try
		{
			Host	host;
			parseHostName(_fileStream, host);	
			_lineNbrEndOfBlock = _lineNbr;
			std::stringstream hostBlock;
			getNextBlock(_fileStream, hostBlock);
			parseHostBlock(hostBlock, host);
			if (_fileStream.eof())
				break;
		}
		catch (const LastBlockException& e)
		{
			break;
		}
		catch (IParseConfigException& e)
		{
			std::string	tmp;
			_fileStream.clear();
			std::getline(_fileStream, tmp);
			LOGE("parsing host at line %d : %s\n -> %sl", std::max(_lineNbr, _lineNbrEndOfBlock), e.what(), &tmp);
			return (1);
		}
	}
	return (0);
}

void	IParseConfig::handleToken(std::stringstream& istream, const std::string& token, Host& host)
{
	if (token == "listen")
		parsePort(istream, host);
	else if (token == "host")
		parseHost(istream, host);
	else if (token == "server_name")
		parseServerName(istream, host);
	else if (token == "error_pages")
		parsePath(istream, host._dir_errors);
	else if (token == "body_max_size")
		parseBodyMaxSize(istream, host);
	else if (token == "location")
		parseLocation(istream, host);
	else
		throw (UnknownTokenException(token));
}

void	IParseConfig::handleLocationToken(std::istream& istream, const std::string& token, t_location& location)
{
	if (token == "root")
		parsePath(istream, location.root);
	else if (token == "methods")
		parseAllowedMethods(istream, location);
	else if (token == "dir_listing")
		parseBoolean(istream, location.dir_listing);
	else if (token == "upload")
		parseBoolean(istream, location.upload);
	else if (token == "default_uri")
		parseUri(istream, location.default_uri);
	else if (token == "upload_root")
		parsePath(istream, location.upload_root);
	else if (token == "exec_cgi")
		parsePath(istream, location.cgi_exec);
	else if (token == "return")
		parseRedirection(istream, location);
	else
		throw (UnknownTokenException(token));
}

void	IParseConfig::parsePort(std::istream& istream, Host& host)
{
	(void)host;
	std::string word;
	if (!(getNextWord(istream, word) & WORD_IS_LAST))
		LOGE("Port missing");
	else
		LOGI("Port : %ss", &word);
}

void	IParseConfig::parseHost(std::istream& istream, Host& host)
{
	(void)host;
	std::string	word;
	if (!(getNextWord(istream, word) & WORD_IS_LAST))
		LOGE("Host missing");
	else
		LOGI("Host : %ss", &word);
}

void	IParseConfig::parseServerName(std::istream& istream, Host& host)
{
	(void)host;
	std::deque<std::string>	serverNames;
	parseValues(istream, serverNames);
	if (serverNames.size() == 0)
		LOGE("Server name missing");
	else
	{
		LOGI("Server names :");
		for (std::deque<std::string>::iterator it = serverNames.begin(); it != serverNames.end(); it++)
			LOGI("	- %ss", &*it);
	}
}

void	IParseConfig::parseBodyMaxSize(std::istream& istream, Host& host)
{
	(void)host;
	std::string word;
	if (!(getNextWord(istream, word) & WORD_IS_LAST))
		LOGE("Max body size is missing");
	else
		LOGI("Max body size : %ss", &word);
}

void	IParseConfig::parseAllowedMethods(std::istream& istream, t_location& location)
{
	(void)location;
	std::deque<std::string>	methods;
	parseValues(istream, methods);
	if (methods.size() == 0)
		LOGE("Method missing");
	else
	{
		LOGI("Allowed methods :");
		for (std::deque<std::string>::iterator it = methods.begin(); it != methods.end(); it++)
			LOGI("	- %ss", &*it);
	}
}

void	IParseConfig::parseRedirection(std::istream& istream, t_location& location)
{
	(void)location;
	std::deque<std::string>	values;
	parseValues(istream, values);
	if (values.size() == 0)
		LOGE("Empty redirection");
	else if (values.size() != 2)
		LOGE("Too many values for redirection");
	else
	{
		LOGI("Redirections :");
		for (std::deque<std::string>::iterator it = values.begin(); it != values.end(); it++)
			LOGI("	- %ss", &*it);
	}
}

void	IParseConfig::parsePath(std::istream& istream, std::string& dest)
{
	if (!(getNextWord(istream, dest) & WORD_IS_LAST))
		LOGE("Path is missing");
	else
		LOGI("Path : %ss", &dest);
}

void	IParseConfig::parseBoolean(std::istream& istream, bool& dest)
{
	(void) dest;
	std::string word;
	if (!(getNextWord(istream, word) & WORD_IS_LAST))
		LOGE("Boolean is missing");
	else
		LOGI("Boolean : %ss", &word);
}

void	IParseConfig::parseUri(std::istream& istream, std::string& dest)
{
	if (!(getNextWord(istream, dest) & WORD_IS_LAST))
		LOGE("Uri is missing");
	else
		LOGI("Uri : %ss", &dest);
}

const char*	IParseConfig::IParseConfigException::what(void) const throw()
{
	return ("parsing error");
}

const char*	IParseConfig::UnclosedQuoteException::what(void) throw()
{
	return ("unclosed quotes");
}

const char*	IParseConfig::UnclosedBlockException::what(void) throw()
{
	return ("block has unclosed braces");
}

// const char*	IParseConfig::UnfinishedCommentException::what(void) const throw()
// {
// 	return ("couldn't find ");
// }

const char*	IParseConfig::StreamException::what(void) throw()
{
	return ("error with the config file stream");
}

const char*	IParseConfig::UnexpectedBraceException::what(void) throw()
{
	return ("closing brace without prior opening brace");
}

const char*	IParseConfig::InvalidFileTypeException::what(void) throw()
{
	return ("invalid config file type");
}

const char*	IParseConfig::FileOpenException::what(void) throw()
{
	return ("could not open config file");
}

const char*	IParseConfig::LastBlockException::what(void) throw()
{
	return ("last block reached");
}

const char*	IParseConfig::MissingSemicolonException::what(void) throw()
{
	return ("missing semicolon (`;')");
}

const char*	IParseConfig::MissingOpeningBraceException::what(void) throw()
{
	return ("missing opening brace `{'");
}

IParseConfig::MissingTokenException::MissingTokenException(const std::string& param) : fieldName(param) {}
IParseConfig::MissingTokenException::~MissingTokenException(void) throw() {}
const char*	IParseConfig::MissingTokenException::what(void) throw()
{
	message = "missing token or field (`" + fieldName + "')";
	return (message.c_str());
	
}

const char*	IParseConfig::MissingTokenException::what(void) const throw() {
	return ("missing token or field");
}

IParseConfig::UnknownTokenException::UnknownTokenException(const std::string& param) : fieldName(param) {}
IParseConfig::UnknownTokenException::~UnknownTokenException(void) throw() {}
const char*	IParseConfig::UnknownTokenException::what(void) throw()
{
	message = "unknown field (`" + fieldName + "')";
	return (message.c_str());
}

const char*	IParseConfig::UnknownTokenException::what(void) const throw() {
	return ("unknown field");
}


IParseConfig::TooManyValuesException::TooManyValuesException(const std::string& param) : fieldName(param) {}
IParseConfig::TooManyValuesException::~TooManyValuesException(void) throw() {}
const char*	IParseConfig::TooManyValuesException::what(void) throw()
{
	message = "too many values for given field (`" + fieldName + "')";
	return (message.c_str());
}

const char*	IParseConfig::TooManyValuesException::what(void) const throw() {
	return ("too many values for given field");
}

IParseConfig::UnexpectedTokenException::UnexpectedTokenException(const std::string& param) : fieldName(param) {}
IParseConfig::UnexpectedTokenException::~UnexpectedTokenException(void) throw() {}
const char*	IParseConfig::UnexpectedTokenException::what(void) throw()
{
	message = "unexpected token (`" + fieldName + "')";
	return (message.c_str());
}

const char*	IParseConfig::UnexpectedTokenException::what(void) const throw() {
	return ("unexpected token");
}
