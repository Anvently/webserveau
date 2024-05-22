#include <IParseConfig.hpp>
#include <sys/stat.h>

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

/// @brief Suppose a comment has been detected from the next character in the stream
/// or because an unescaped ```#``` was read.
/// Advance the stream to the position following the next ```\\n``` or ```\\0```.
/// @param istream 
void IParseConfig::skipComment(std::istream& istream)
{
	char	c;

	c = istream.peek();
	while (c != EOF)
	{
		c = istream.get();
		if (c == '\n')
		{
			_lineNbr++;
			return;
		}
	}
}

/// @brief Skip next spaces character in stream and set stream position
/// at the first non space character or ```EOF```.
/// @param istream 
/// @return 
inline void	IParseConfig::skipSpace(std::istream& istream)
{
	char	c;

	while (std::isspace(istream.peek())) {
		c = istream.get();
		if (c == '\n')
			_lineNbr++;
	}
}

/// @brief Check if the next word in the stream is a semicolon.
/// If so skip it, else stop at the first non-whitespace character.
/// @param istream 
/// @return 
inline bool	IParseConfig::checkSemiColon(std::istream& istream)
{
	skipSpace(istream);
	if (istream.peek() == ';')
	{
		istream.get();
		return (true);
	}
	return (false);
}

/// @brief Read the next word in the stream and assign it to ```word``` string if it
/// is indeed a word. A valid word is delimited by [ ```whitespace``` ```;``` ```{```
/// ```}``` ```EOF``` ].
/// Check and skip for leading spaces and/or comments.
/// Expand quotes as single word. Handles escaping character with ```\\```.
/// @param istream 
/// @param word 
/// @return  ```0``` if a word was successfully extracted. If a delimiter
/// (excepting whitespaces) without preceding word is found, return the delimiter.
int	IParseConfig::	getNextWord(std::istream& istream, std::string& word)
{
	char	cur, next;
	bool	isEscaped = false;

	skipSpace(istream);
	next = istream.peek();
	while (isEscaped || (std::isspace(next) == false && next != EOF && next != ';' && next != '{' && next != '}'))
	{
		cur = istream.get();
		if (isEscaped == true)
		{
			word += cur;
			isEscaped = false;
		}
		else if (cur == '\\')
			isEscaped = true;
		else if (cur == '"')
			parseQuote(istream, word);
		else if ((cur == '/' && (word.size() == 0 && istream.peek() == '/')) || cur == '#')
		{
			skipComment(istream);
			skipSpace(istream);
		}
		else
			word += cur;
		next = istream.peek();
	}
	if (word.size())
		return (0);
	return (next);
}

/// @brief Parse ```maxNbr``` words in ```istream```
/// @param istream 
/// @param words 
/// @param maxNbr 
void	IParseConfig::parseValues(std::istream& istream, std::deque<std::string>& words, int maxNbr = INT32_MAX)
{
	std::string	word;
	int			i = 0, ret = 0;

	do
	{
		word.clear();
		ret = getNextWord(istream, word);
		if (ret == 0)
			words.push_front(word);
		i++;
	} while (ret == 0 && i < maxNbr);
}

void	IParseConfig::parseHostBlock(std::stringstream& hostBlock, Host& host)
{
	std::string				token;
	int						ret;
	
	try
	{
		do
		{
			token.clear();
			ret = getNextWord(hostBlock, token);
			if (ret == 0)
				handleToken(hostBlock, token, host);
			else if (ret == EOF)
				break;
			else
				throw (UnexpectedTokenException(token));
			if (checkSemiColon(hostBlock) == false)
				throw (MissingSemicolonException());
		} while (ret == 0);
	}
	catch (IParseConfigException& e)
	{
		std::string	tmp;
		if (hostBlock.good())
			tmp = hostBlock.str().substr(hostBlock.tellg());
		else
			tmp = "[End of block]";
		LOGE("parsing host (%ss) configuration at line %d : %s\n -> ...%sl...", &host._name, _lineNbr, e.what(), &tmp);
		_lineNbr = _lineNbrEndOfBlock;
	}
}

void	IParseConfig::parseCGIConfig(std::stringstream& istream, Host& host)
{
	std::deque<std::string>	locationsName;
	CGIConfig				cgiConfig;

	parseValues(istream, locationsName); //May need to check what happens gere
	std::stringstream	CGIConfigStream;
	_lineNbrEndOfBlock = _lineNbr;
	getNextBlock(istream, CGIConfigStream);
	std::string tmp = CGIConfigStream.str();
	try {
		parseCGIConfigBlock(CGIConfigStream, cgiConfig);
	}
	catch (const LastBlockException& e) {
		throw (MissingOpeningBraceException());
	}
	catch (IParseConfigException& e)
	{
		std::string	tmp;
		if (CGIConfigStream.good())
			tmp = CGIConfigStream.str().substr(CGIConfigStream.tellg());
		else
			tmp = "[End of block]";
		LOGE("parsing host (%ss) cgi config at line %d : %s\n -> %sl", &host._name, _lineNbr, e.what(), &tmp);
		_lineNbr = _lineNbrEndOfBlock;
	}
}

void	IParseConfig::parseLocation(std::stringstream& istream, Host& host)
{
	std::deque<std::string>	locationsName;
	Location				location;

	parseValues(istream, locationsName); //May need to check what happens gere
	std::stringstream	locationStream;
	_lineNbrEndOfBlock = _lineNbr;
	getNextBlock(istream, locationStream);
	std::string tmp = locationStream.str();
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
			tmp = "[End of block]";
		LOGE("parsing host (%ss) location at line %d : %s\n -> %sl", &host._name, _lineNbr, e.what(), &tmp);
		_lineNbr = _lineNbrEndOfBlock;
	}
}

void	IParseConfig::parseLocationBlock(std::istream& locationBlock, Location& location)
{
	std::string				token;
	int						ret;
	
	do
	{
		token.clear();
		ret = getNextWord(locationBlock, token);
		if (ret == 0)
			handleLocationToken(locationBlock, token, location);
		else if (ret == EOF)
			break;
		else
			throw (UnexpectedTokenException(token));
		if (checkSemiColon(locationBlock) == false)
			throw (MissingSemicolonException());
	} while (ret == 0);
}

void	IParseConfig::parseCGIConfigBlock(std::istream& cgiStream, CGIConfig& cgiConfig)
{
	std::string				token;
	int						ret;
	
	do
	{
		token.clear();
		ret = getNextWord(cgiStream, token);
		if (ret == 0)
			handleCGIConfigToken(cgiStream, token, cgiConfig);
		else if (ret == EOF)
			break;
		else
			throw (UnexpectedTokenException(token));
		if (checkSemiColon(cgiStream) == false)
			throw (MissingSemicolonException());
	} while (ret == 0);
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

	ret = getNextWord(istream, word);
	if (ret == EOF)
		throw (LastBlockException());
	else if (ret)
		throw (UnexpectedTokenException(word));
	host._name = word;
	word.clear();
	ret = getNextWord(istream, word);
	if (ret == EOF)
		throw (MissingOpeningBraceException());
	else if (ret != '{')
		throw (UnexpectedTokenException(word + "instead of {"));
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
	else if (token == "cgi")
		parseCGIConfig(istream, host);
	else
		throw (UnknownTokenException(token));
}

void	IParseConfig::handleCGIConfigToken(std::istream& istream,  const std::string& token, CGIConfig& location)
{
	if (token == "root")
		parsePath(istream, location.root);
	else if (token == "methods")
		parseAllowedMethods(istream, location.methods);
	else if (token == "exec")
		parsePath(istream, location.exec);
	else
		throw (UnknownTokenException(token));
}

void	IParseConfig::handleLocationToken(std::istream& istream, const std::string& token, Location& location)
{
	if (token == "root")
		parsePath(istream, location.root);
	else if (token == "methods")
		parseAllowedMethods(istream, location.methods);
	else if (token == "dir_listing")
		parseBoolean(istream, location.dir_listing);
	else if (token == "upload")
		parseBoolean(istream, location.upload);
	else if (token == "default_uri")
		parseUri(istream, location.default_uri);
	else if (token == "upload_root")
		parsePath(istream, location.upload_root);
	else if (token == "return")
		parseRedirection(istream, location);
	else
		throw (UnknownTokenException(token));
}

void	IParseConfig::parsePort(std::istream& istream, Host& host)
{
	(void)host;
	std::string word;
	if (getNextWord(istream, word))
		LOGE("Port missing");
	else
		LOGI("Port : %ss", &word);
}

void	IParseConfig::parseHost(std::istream& istream, Host& host)
{
	(void)host;
	std::string	word;
	if (getNextWord(istream, word))
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
	if (getNextWord(istream, word))
		LOGE("Max body size is missing");
	else
		LOGI("Max body size : %ss", &word);
}

void	IParseConfig::parseAllowedMethods(std::istream& istream, bool (&dest)[METHODS_NBR])
{
	(void)dest;
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

void	IParseConfig::parseRedirection(std::istream& istream, Location& location)
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
	if (getNextWord(istream, dest))
		LOGE("Path is missing");
	else
		LOGI("Path : %ss", &dest);
}

void	IParseConfig::parseBoolean(std::istream& istream, bool& dest)
{
	(void) dest;
	std::string word;
	if (getNextWord(istream, word))
		LOGE("Boolean is missing");
	else
		LOGI("Boolean : %ss", &word);
}

void	IParseConfig::parseUri(std::istream& istream, std::string& dest)
{
	if (getNextWord(istream, dest))
		LOGE("Uri is missing");
	else
		LOGI("Uri : %ss", &dest);
}
