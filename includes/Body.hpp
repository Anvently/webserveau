#ifndef BODY_HPP
#define BODY_HPP

#include <string>
#include <iostream>
#include <Header.hpp>

std::string	IntToString(int x, int base);
int	getInt(std::string &str, int base, int &res);
bool nocase_string_eq(const std::string& a, const std::string& b);


class	Body{

	private:
		int			_length;
		int			_content_length; // if the header content length was present will be positive else -1 ?
		int			_max_body_size;
		bool		_chuncked;
		int			_error_num;

		Header		*_header;

		bool		_is_complete;

		int	_parseChunked(std::string &buffer);
		int	_parseMesured(std::string &buffer);
		Body();

	public:

		Body(Header *header, int max_size);
		~Body();

		int	parseInput(std::string &buffer); //add the buffer to the body return 0 or an error code
		int	getChunkedLine(std::string &buffer);


		std::string	getHeader(std::string &key);

		class	SizeRelatedHeaderMissing: public std::exception
		{
			public:
				SizeRelatedHeaderMissing();
				const char* what() const throw();
		};
};

#endif
