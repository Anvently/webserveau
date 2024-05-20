#include <IParseConfig.hpp>
#include <sys/stat.h>

// #define WORD_IS_LAST 1
// #define WORD_IS_COMMENT 2
// #define WORD_IS_CITATION 3

#define WORD_IS_BRACE (1 << 3)
#define WORD_IS_CITATION (1 << 2)
#define WORD_IS_LAST (1 << 1)
#define WORD_IS_COMMENT (1 << 0)

std::ifstream	IParseConfig::_fileStream;
std::string		IParseConfig::_lineBuffer;
int				IParseConfig::_lineNbr = 0;
int				IParseConfig::_lineNbrFile = 0;

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
		_lineNbr++;
		start = _lineBuffer.begin();
		end = _lineBuffer.end();
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
		if (it != _lineBuffer.end() && std::distance(it, _lineBuffer.end()) > 1)
		{
			if (istream.eof())
			{
				istream.clear();
				istream.seekg(istream.tellg() - std::distance(it, _lineBuffer.end() - 1));
			}
			else
				istream.seekg(istream.tellg() - std::distance(it, _lineBuffer.end()));
			_lineNbr--;
		}
		if (enclosedDepth != 0 || (enclosedDepth == 0 && (start != _lineBuffer.begin() || end != _lineBuffer.end())))
		{
			ostream << _lineBuffer.substr(std::distance(_lineBuffer.begin(), start), std::distance(start, end));
			if (enclosedDepth)
				ostream << '\n';
		}
		ostream.flush();
	} while (istream.good() && istream.eof() == false && it == _lineBuffer.end());
	if (istream.bad() && istream.eof() == false)
		throw (FileStreamException());
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
// 		throw(FileStreamException());
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
	throw (FileStreamException());
}

void	IParseConfig::handleToken(std::istream& istream, const std::string& token, Host& host)
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
		throw (UnknownTokenException());
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
		throw (UnknownTokenException());
}

/// @brief Read the next word in the stream and assign it to ```word``` string,
/// checking if it's a comment, if there are quote to expand, and if the word ends
/// with a semicolon.
/// If word is (or start with) a comment token, stream is readed until ```\\n```
/// or ```\\0```, and ```WORD_IS_COMMENT``` is returned. If word is a quoted sequence, the
/// full sequence is appended to word. If word ends with a semicolon (```;```).
/// then ```WORD_IS_LAST``` is returned. If word ends with a brace ```{}```, stream cursor
/// is moved back of one char and ```WORD_IS_LAST``` is returned.
/// ```WORD_IS_LAST``` and ```WORD_IS_CITATION``` can be ORed.
/// @param istream 
/// @param word ```0``` if a single and normal word was read.
int	IParseConfig::parseWord(std::istream& istream, std::string& word)
{
	char	c;
	bool	isEscaped = false;
	int		ret = 0;

	c = istream.get();
	if ((c == '/' && (word.size() == 1 && word.front() == '/')) || c == '#')
	{
		parseComment(istream);
		word.clear();
		return (WORD_IS_COMMENT);
	}
	while ((std::isspace(c) == false && c != ';' && c != '{' && c != '}') || isEscaped == true)
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
		else
			word += c;
		c = istream.get();
		if (istream.eof())
			break;
	}
	if (c == ';')
		ret |= WORD_IS_LAST;
	else if (c == '{' || c== '}')
	{
		istream.seekg(-1);
		if (word.size() > 0)
			ret |= WORD_IS_LAST;
		else
			ret |= WORD_IS_BRACE;
	}
	else if (c == '\n')
		_lineNbr++;
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
		ret = parseWord(istream, word);
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

void	IParseConfig::parseHostBlock(std::istream& hostBlock, Host& host)
{
	std::string				token;
	int						ret;
	
	try
	{
		while (hostBlock.good())
		{
			ret = parseWord(hostBlock, token); 
			if (ret == 0)
				handleToken(hostBlock, token, host);
			else if ((ret & WORD_IS_COMMENT) == 0)
				throw (UnexpectedTokenException());
		}
	}
	catch (const std::exception& e)
	{
		LOGE("parsing host (%ss) configuration at line %d : %s", &host._name, _lineNbrFile + _lineNbr, e.what());
	}
}


void	IParseConfig::parseLocation(std::istream& istream, Host& host)
{
	std::deque<std::string>	locationsName;
	t_location				location;

	parseValues(istream, locationsName); //May need to check what happens gere
	std::stringstream	locationStream;
	getNextBlock(istream, locationStream);
	try {
		parseLocationBlock(locationStream, location);
	}
	catch (const std::exception& e)
	{
		LOGE("parsing host (%ss) location at line %d : %s", &host._name, _lineNbrFile + _lineNbr, e.what());
	}
}

void	IParseConfig::parseLocationBlock(std::istream& locationBlock, t_location& location)
{
	std::string				token;
	int						ret;
	
	while (locationBlock.good())
	{
		ret = parseWord(locationBlock, token); 
		if (ret == 0)
			handleLocationToken(locationBlock, token, location);
		else if ((ret & WORD_IS_COMMENT) == 0)
			throw (UnexpectedTokenException());
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
	catch (const FileException& e)
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

	while (_fileStream.good())
	{
		word.clear();
		ret = parseWord(_fileStream, word);
		if (ret == WORD_IS_COMMENT)
			continue;
		else if (ret & WORD_IS_CITATION)
			throw (UnexpectedTokenException());
		host._name = word;
		if (!(parseWord(_fileStream, word) & WORD_IS_BRACE))
			throw (UnexpectedTokenException());
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
			std::stringstream hostBlock;
			getNextBlock(_fileStream, hostBlock);
			_lineNbrFile += _lineNbr;
			_lineNbr = 0;
			parseHostBlock(hostBlock, host);
			if (_fileStream.eof())
				break;
		}
		catch (const LastBlockException& e)
		{
			break;
		}
		catch (const std::exception& e)
		{
			// LOGE("%d | %d", _lineNbr, _lineNbrFile);
			LOGE("parsing host at line %d : %s\n -> %sh", _lineNbrFile + _lineNbr, e.what(), &_lineBuffer);
			return (1);
		}
	}
	return (0);
}

const char*	IParseConfig::UnclosedQuoteException::what(void) const throw()
{
	return ("unclosed quotes");
}

const char*	IParseConfig::UnclosedBlockException::what(void) const throw()
{
	return ("block has unclosed braces");
}

const char*	IParseConfig::FileStreamException::what(void) const throw()
{
	return ("error with the config file stream");
}

const char*	IParseConfig::UnexpectedBraceException::what(void) const throw()
{
	return ("closing brace without prior opening brace");
}

const char*	IParseConfig::InvalidFileTypeException::what(void) const throw()
{
	return ("invalid config file type");
}

const char*	IParseConfig::FileOpenException::what(void) const throw()
{
	return ("could not open config file");
}

const char*	IParseConfig::LastBlockException::what(void) const throw()
{
	return ("last block reached");
}

const char*	IParseConfig::MissingSemicolonException::what(void) const throw()
{
	return ("missing semicolon (`;')");
}

const char*	IParseConfig::UnexpectedTokenException::what(void) const throw()
{
	return ("unexpected field name format");
}

const char*	IParseConfig::UnknownTokenException::what(void) const throw()
{
	return ("unknow field name");
}

// int	IParseConfig::parseBraceBlock(const std::string& block)
// {
// 	std::string::iterator	it, start, end;
// }
