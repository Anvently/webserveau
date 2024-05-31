#include <string>
#include <map>
#include <vector>
#include <algorithm>
#include <iostream>

/// @brief Compare a token formed expression including ```*``` symbol with an uri.
/// @param location 
/// @param uri 
/// @return 
bool	isUriMatch(const std::string& expression, const std::string& uri) {
	std::vector<std::string>	tokens;
	size_t	i = 0;

	while (expression[i]) {
		if (expression[i] == '*') {
			tokens.push_back("*");
			i++;
		}
		else {
			int begin = i;
			while (expression[i] && expression[i] != '*')
				i++;
			tokens.push_back(expression.substr(begin, i - begin));
		}
	}
	i = 0;
	std::vector<std::string>::iterator it = tokens.begin();
	while (uri[i] && it != tokens.end()) {
		if (*it == "*") {
			if (++it == tokens.end())
				return (true);
			if ((i = uri.find(*it, i)) == std::string::npos)
				return (false);
			i += it->length();
		} else {
			if (uri.compare(i, it->length(), it->c_str()) != 0)
				return (false);
			i += it->length();
		}
	}
	if (it != tokens.end() && uri[i] == '\0') {
		if (*it != "*")
			return (false);
		return (true);
	}
	return (false);
}

static void	tokenize(const std::string& expression, std::vector<std::string>& tokenVec)
{
	int	i = 0;

	while (expression[i]) {
		if (expression[i] == '*') {
			tokenVec.push_back("*");
			i++;
		}
		else {
			int begin = i;
			while (expression[i] && expression[i] != '*')
				i++;
			tokenVec.push_back(expression.substr(begin, i - begin));
		}
	}
}

static bool	isMatch(const std::string& uri, std::vector<std::string>& tokenVec)
{
	size_t	i = 0;

	std::vector<std::string>::iterator it = tokenVec.begin();
	if (it == tokenVec.end())
		return (false);
	while (uri[i]) {
		if (*it == "*") {
			if (++it == tokenVec.end())
				return (true);
			if ((i = uri.find(*it, i)) == std::string::npos)
				return (false);
			i += it->length();
		} else {
			if (uri.compare(i, it->length(), it->c_str()) != 0)
				return (false);
			i += it->length();
		}
		it++;
		if (it == tokenVec.end()) {
			if (uri[i])
				return (false);
			return(true);
		}
	}
	if (it != tokenVec.end()) {
		if (*it++ != "*" || it != tokenVec.end())
			return (false);
		// if (it != tokenVec.end())
		// 	return (false);
	}
	return (true);
}

/// @brief Compare a token formed expression including ```*``` symbol with an uri.
/// @param location 
/// @param uri 
/// @return 
bool	isBetterMatch(const std::string& uriRef, const std::string& expressionA, const std::string* expressionB = NULL)
{
	std::string	uri = uriRef;
	std::vector<std::string>	tokensA, tokensB;
	
	tokenize(expressionA, tokensA);
	if (isMatch(uri, tokensA) == false)
		return (false);
	if (expressionB == NULL)
		return (true);
	tokenize(*expressionB, tokensB);
	std::vector<std::string>::const_reverse_iterator	itA = tokensA.rbegin(), \
														itB = tokensB.rbegin();
	while (itA != tokensA.rend() && itB != tokensB.rend()) {
		if (*itA == "*") {
			if (*itB == "*")
				itA++, itB++;
			else
				return (false);
		} else if (*itB == "*")
			return (true);
		else {
			int	posA = uri.rfind(*itA), posB = uri.rfind(*itB);
			std::cout << "Uri = " << uri << std::endl;
			std::cout << "A = " << *itA << std::endl;
			std::cout << "B = " << *itB << std::endl;
			std::cout << "A ends at = " << posA + itA->length() << std::endl;
			std::cout << "B ends at = " << posB + itB->length() << std::endl;
			int	diff = (posA + itA->length()) - (posB + itB->length());
			std::cout << "Diff = " << diff << std::endl;
			if (diff > 0)
				return (true);
			else if (diff < 0)
				return (false);
			else if (itA->length() > itB->length())
				return (true);
			else if (itA->length() < itB->length())
				return (false);
			uri = uri.substr(0, posA);
			itA++, itB++;
		}
	}
	if (itA == tokensA.rend())
		return (false);
	return (true);
}

struct	Location {
	std::string	_name;
	Location(const std::string& name);
};

Location::Location(const std::string& name) : _name(name) {}

template <typename T>
T*	getObjectMatch(const typename std::map<std::string, T*>& map, const std::string& uri) {
	const typename std::pair<const std::string, T*>*	bestMatch = NULL;

	for (typename std::map<std::string, T*>::const_iterator it = map.begin(); it != map.end(); it++)
	{
		std::cout << "checking match " << it->first << std::endl;
		if (isBetterMatch(uri, it->first, (bestMatch ? &bestMatch->first : NULL))) {
			bestMatch = &*it;
			std::cout << "new best match is " << bestMatch->first << std::endl;
		}
	}
	if (bestMatch)
		return (bestMatch->second);
	return (NULL);

}

int	main(void) {
	std::string	uri = "root/imgs/root/serv/test/imgs/img1.png";

	Location	loc1("loc1");
	Location	loc2("loc2");
	Location	loc3("loc3");
	Location	loc4("loc4");
	Location	loc5("loc5");
	Location	loc6("loc6");
	Location	loc7("loc7");
	Location	loc8("loc8");
	Location	loc9("loc9");
	Location	loc10("loc10");
	Location	loc11("loc11");
	Location	loc12("loc12");
	Location	loc13("loc13");
	std::map<std::string, Location*>	locMap;
	locMap["*/root/*/tes*"] = &loc12;
	locMap["*/test/*"] = &loc11	;
	locMap["*/imgs/*"] = &loc10;
	locMap["*/imgs/*/imgs/*"] = &loc9;
	locMap["*root/imgs/*/imgs/*"] = &loc8;
	locMap["*/test/imgs/*"] = &loc7;
	locMap["*/*mg1.pn*"] = &loc6;
	locMap["*/img1.pn*"] = &loc5;
	locMap["*.png*"] = &loc4;
	locMap["*.png"] = &loc3;
	locMap["*/*.png"] = &loc2;
	locMap["*/img1.png"] = &loc1;	
	
	Location*	bestMatch = getObjectMatch(locMap, uri);
	if (bestMatch)
		std::cout << "best match is " << bestMatch->_name << std::endl;
	else
		std::cout << "no best match\n";
	return (0);
}
