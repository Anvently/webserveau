#include <IParseConfig.hpp>
#include <sys/stat.h>

std::ifstream	IParseConfig::_fileStream;
std::string		IParseConfig::_lineBuffer;
int				IParseConfig::_fileLineNum = 0;
int				IParseConfig::_bufferLineNbr = 0;

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
std::string	IParseConfig::getNextBlock(std::istream& stream)
{
	std::string				block;
	int						enclosedDepth = 0;
	bool					isEscaped = false, isQuoted = false;
	std::string::iterator	it, start, end;

	do
	{
		std::getline(stream, _lineBuffer);
		_fileLineNum++;
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
			if (stream.eof())
			{
				stream.clear();
				stream.seekg(stream.tellg() - std::distance(it, _lineBuffer.end() - 1));
			}
			else
				stream.seekg(stream.tellg() - std::distance(it, _lineBuffer.end()));
			_fileLineNum--;
		}
		if (enclosedDepth != 0 || (enclosedDepth == 0 && (start != _lineBuffer.begin() || end != _lineBuffer.end())))
			block += _lineBuffer.substr(std::distance(_lineBuffer.begin(), start), std::distance(start, end));
	} while (stream.good() && stream.eof() == false && it == _lineBuffer.end());
	if (stream.bad() && stream.eof() == false)
		throw (FileStreamException());
	if (enclosedDepth > 0)
		throw (UnclosedBlockException());
	if (stream.eof() && end == _lineBuffer.end())
		throw (LastBlockException());
	return (block);
}

/// @brief Return a string containing the next block found in config file.
/// Block  start at the first ```{``` encountered.
/// Block is defined by content delimited with ```{}```. Nested block are ignored.
/// ```\\``` is an escape character, ```{}``` within quote (```"```) are ignored.
/// Syntax error will raise exceptions. When the function reach eof without finding
/// any block ```LastBlockException``` is raised.
/// @param  
/// @return 
std::string		IParseConfig::getNextServerBlock(void)
{
	if (_fileStream.is_open() == false || _fileStream.good() == false)
		throw(FileStreamException());
	return (getNextBlock(_fileStream));
}

void	IParseConfig::parseServerBlock(std::istringstream& serverBlock)
{
	// std::string test = "	listen 8080;	host 127.0.0.1;	server_name server1 another.name.fr;	error_pages /errors/my_custom_errors1/;	client_max_size 1024;	location /{ # ce loc block est obligatoireroot /var/www/server1/ #obligatoire		methods GET POST DELETE;		dir_listing off;		default_uri index_images.html;		upload off;	}	location /images/ {		root /var/www/server1/data/; //override the root above?		methods GET POST DELETE;		dir_listing on;		default_uri index_images.html//error?;		upload on;upload_root /var/www/server1/tmp/;	}	location /tmp/ {	}	location *.php{		root /var/www/server1/cgi/php/;		methods POST GET;		exec_cgi php;		upload off; // mandatory	}";
	// std::istringstream	stream(test);
	// LOGD("BLOCK = |%ss|", &test);
	while (1)
	{
		try
		{
			// std::string block = getNextBlock(stream);
			std::string block = getNextBlock(serverBlock);
			LOGD("location_block = |%ss|", &block);
			if (serverBlock.eof())
				break;
		}
		catch (const LastBlockException& e)
		{
			LOGI("%s", e.what());
			break;
		}
		catch (const std::exception& e)
		{
			LOGE("at line %d : %s", _fileLineNum, e.what());
			// return (1);
		}
	}
	// return (0);
}


void	IParseConfig::parseLocation(t_location* location, const std::string& block)
{
	(void)location;
	(void)block;
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
	// std::istringstream	stream;
	// parseServerBlock(stream);
	// return (0);
	while (1)
	{
		try
		{
			std::string block = getNextServerBlock();
			LOGD("BLOCK = |%ss|", &block);
			std::istringstream	stream(block);
			parseServerBlock(stream);
			if (_fileStream.eof())
				break;
		}
		catch (const LastBlockException& e)
		{
			LOGI("%s", e.what());
			break;
		}
		catch (const std::exception& e)
		{
			LOGE("at line %d : %s", _fileLineNum, e.what());
			return (1);
		}
	}
	return (0);
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

// int	IParseConfig::parseBraceBlock(const std::string& block)
// {
// 	std::string::iterator	it, start, end;
// }
