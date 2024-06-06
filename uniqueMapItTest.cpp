#include "includes/UniqueValuesMapIterator.tpp"
#include "includes/ListenServer.hpp"
#include <iostream>
#include <string>
#include <Host.hpp>


Host*	insert(std::list<Host>& list, const std::string& name) {
	Host	host;
	Host&	ref = *list.insert(list.end(), host);
	return (&ref);
}

int	main(void) {
	
	ListenServer*	test = ListenServer::addServer("127.0.0.1", "8080");

	std::list<Host>	list;
	Host*	host0 = insert(list, "host0");
	Host*	host1 = insert(list, "host1");

	test->_hostMap["another.name.en"] = host0;
	test->_hostMap["another.name.fr"] = host1;
	test->_hostMap["server1"] = host1;
	test->_hostMap["server2"] = host0;
	// map["key4"] = host0;
	// map["key4"] = host0;
	// // UniqueValuesMapIterator_const<std::string, Host*> it(map.begin());
	// ++it;
	// ++it;
	// ++it;
	for (UniqueValuesMapIterator_const<std::string, Host*> it(test->_hostMap.begin()); it != test->_hostMap.end(); it++) {
		std::cout << it->first << std::endl;
	}
}