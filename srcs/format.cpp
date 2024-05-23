#include <errno.h>
#include <stdlib.h>
#include <string>
#include <algorithm>
#include <iterator>
#include <cctype>
#include <iostream>
#include <sstream>

static const std::string deci = "0123456789";
static const std::string hexa = "0123456789ABCDEF";

char	to_upper(unsigned char c)
{
	return (std::toupper(c));
}

int	getInt(std::string &str, int base, int &res)
{
	std::transform(str.begin(), str.end(), str.begin(), to_upper);
	std::string	sbase;
	const char	*p;
	std::string::size_type idx = 0;
	switch (base){
		case 10:
			sbase = deci;
			break ;
		case 16:
			sbase = hexa;
			break ;
	}
	while (str[idx] == 32 || str[idx] == 9)
		idx++;
	if (str[idx] == '+' || str[idx] == '-')
		idx++;
	if (idx == str.size())
		return (1);
	if (str.find_first_not_of(sbase, idx) != std::string::npos)
		return (1);
	p = str.c_str();
	errno = 0;
	res = strtol(p, NULL, base);
	if (errno)
		return (1);
	return (0);
}


std::string	IntToString(int x, int base)
{
	std::ostringstream ss;
	if (base == 16)
		ss << std::hex;
	ss << x;
	return (ss.str());
}

bool ichar_equals(char a, char b)
{
    return std::tolower(static_cast<unsigned char>(a)) ==
           std::tolower(static_cast<unsigned char>(b));
}

bool nocase_string_eq(const std::string& a, const std::string& b)
{
    return a.size() == b.size() &&
           std::equal(a.begin(), a.end(), b.begin(), ichar_equals);
}
