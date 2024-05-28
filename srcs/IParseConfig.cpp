#include <IParseConfig.hpp>
#include <sys/stat.h>
#include <algorithm>
#include <Request.hpp>

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

void	IParseConfig::parseBlock(std::stringstream& blockStream, void* obj, handleTokenFunc handler)
{
	std::string				token;
	int						ret;

	do
	{
		token.clear();
		ret = getNextWord(blockStream, token);
		if (ret == 0)
			(*handler)(blockStream, token, obj);
		else if (ret == EOF)
			break;
		else
			throw (UnexpectedTokenException(token));
		if (checkSemiColon(blockStream) == false)
			throw (MissingSemicolonException());
	} while (ret == 0);
}

void	IParseConfig::parseHost(std::istream& istream)
{
	Host				host;
	std::stringstream	hostStream;

	_lineNbrEndOfBlock = _lineNbr;
	getNextBlock(istream, hostStream);
	try
	{
		parseBlock(hostStream, &host, handleHostToken);
		Host::addHost(host);
	}
	catch (IParseConfigException& e)
	{
		std::string	tmp;
		if (hostStream.good())
			tmp = hostStream.str().substr(hostStream.tellg());
		else
			tmp = "[End of block]";
		LOGE("parsing host (%ss) configuration at line %d : %s\n -> ...%sl...", &host._addr, _lineNbr, e.what(), &tmp);
		_lineNbr = _lineNbrEndOfBlock;
	}
}

void	IParseConfig::parseCGIConfig(std::stringstream& istream, Host& host)
{
	std::deque<std::string>	cgiConfigNames;
	CGIConfig				cgiConfig;

	parseValues(istream, cgiConfigNames); //May need to check what happens gere
	std::stringstream	CGIConfigStream;
	_lineNbrEndOfBlock = _lineNbr;
	getNextBlock(istream, CGIConfigStream);
	try {
		parseBlock(CGIConfigStream, &cgiConfig, handleCGIConfigToken);
		host.addCGIConfig(cgiConfigNames, cgiConfig);
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
		LOGE("parsing host (%ss) cgi config at line %d : %s\n -> %sl", &host._addr, _lineNbr, e.what(), &tmp);
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
		parseBlock(locationStream, &location, handleLocationToken);
		host.addLocation(locationsName, location);
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
		LOGE("parsing host (%ss) location at line %d : %s\n -> %sl", &host._addr, _lineNbr, e.what(), &tmp);
		_lineNbr = _lineNbrEndOfBlock;
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
	int	ret;
	if (openFile(path))
		return (1);
	try
	{
		do
		{
			std::string 	token;
			ret = getNextWord(_fileStream, token);
			if (ret == 0)
				handleConfigToken(token);
			else if (ret == EOF)
				break;
			else
				throw (UnexpectedTokenException(token));
			if (checkSemiColon(_fileStream) == false)
				throw (MissingSemicolonException());
		} while (ret == 0);
	}
	catch (const LastBlockException& e) {}
	catch (IParseConfigException& e)
	{
		std::string	tmp;
		_fileStream.clear();
		std::getline(_fileStream, tmp);
		LOGE("parsing host at line %d : %s\n -> %sl", std::max(_lineNbr, _lineNbrEndOfBlock), e.what(), &tmp);
		return (1);
	}
	return (0);
}

void	IParseConfig::handleConfigToken(const std::string& token)
{
	if (token == "server")
		parseHost(_fileStream);
	else
		throw (UnknownTokenException(token));
}

void	IParseConfig::handleHostToken(std::stringstream& istream, const std::string& token, void* hostPtr)
{
	Host&	host = *(Host*)hostPtr;

	if (token == "listen")
		parsePort(istream, host);
	else if (token == "host")
		parseHostName(istream, host);
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

void	IParseConfig::handleCGIConfigToken(std::stringstream& istream,  const std::string& token, void* CGIConfigPtr)
{
	CGIConfig&	cgi = *(CGIConfig*)CGIConfigPtr;
	if (token == "root")
		parsePath(istream, cgi.root);
	else if (token == "methods")
		parseAllowedMethods(istream, cgi.methods);
	else if (token == "exec")
		parsePath(istream, cgi.exec);
	else
		throw (UnknownTokenException(token));
}

void	IParseConfig::handleLocationToken(std::stringstream& istream, const std::string& token, void* locationPtr)
{
	Location&	location = *(Location*)locationPtr;
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
	int			port;

	if (getNextWord(istream, host._port)) {
		LOGE("Port missing");
		return ;
	}
	if (getInt(host._port, 10, port))
		LOGE("Invalid port");
}

void	IParseConfig::parseHostName(std::istream& istream, Host& host)
{
	std::string	word;
	if (getNextWord(istream, host._addr)) {
		LOGE("Host missing");
		return;
	}
}

void	IParseConfig::parseServerName(std::istream& istream, Host& host)
{
	parseValues(istream, host._server_names);
	if (host._server_names.size() == 0) {
		LOGE("Server name missing");
		return;
	}
	// else
	// {
	// 	LOGI("Server names :");
	// 	for (std::vector<std::string>::iterator it = host._server_names.begin(); it != host._server_names.end(); it++)
	// 		LOGI("	- %ss", &*it);
	// }
}

void	IParseConfig::parseBodyMaxSize(std::istream& istream, Host& host)
{
	std::string word;
	if (getNextWord(istream, word)) {
		LOGE("Max body size missing");
		return ;
	}
	if (getInt(word, 10, host._body_max_size))
		LOGE("Invalid max body size");
}

void	IParseConfig::parseAllowedMethods(std::istream& istream, bool (&dest)[METHOD_NBR])
{
	std::deque<std::string>	methods;
	parseValues(istream, methods);
	if (methods.size() == 0) {
		LOGE("Method missing");
		return;
	}
	for (std::deque<std::string>::iterator it = methods.begin(); it != methods.end(); it++)
	{
		int index = getMethodIndex(*it);
		if (index < 0){
			LOGE("Invalid method");
			return;
		}
		dest[index] = true;
	}
	// LOGI("Allowed methods :");
	// for (std::deque<std::string>::iterator it = methods.begin(); it != methods.end(); it++)
	// 	LOGI("	- %ss", &*it);
}

void	IParseConfig::parseRedirection(std::istream& istream, Location& location)
{
	int	errorCode;
	std::deque<std::string>	values;

	parseValues(istream, values);
	if (values.size() == 0)
		LOGE("Empty redirection");
	else if (values.size() != 2)
		LOGE("Incorrect number of values for redirection");
	else if (getInt(values.at(0), 10, errorCode))
		LOGE("Invalid redirection code");
	else
	{
		location.addr_redir[errorCode] = values.at(1);
		// LOGI("Redirections :");
		// for (std::deque<std::string>::iterator it = values.begin(); it != values.end(); it++)
			// LOGI("	- %ss", &*it);
	}
}

void	IParseConfig::parsePath(std::istream& istream, std::string& dest, const char* id)
{
	if (getNextWord(istream, dest)) {
		LOGE("%s is missing", id);
		return;
	}
}

void	IParseConfig::parseBoolean(std::istream& istream, bool& dest, const char* id)
{
	std::string word;
	if (getNextWord(istream, word)) {
		LOGE("%s is missing", id);
		return;
	}
	if (word == "on")
		dest = true;
	else if (word == "off")
		dest = false;
	else
		LOGE("invalid value for %s", id);
}

void	IParseConfig::parseUri(std::istream& istream, std::string& dest, const char* id)
{
	if (getNextWord(istream, dest)) {
		LOGE("%s is missing", id);
		return;
	}
}
