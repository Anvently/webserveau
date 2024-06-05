#include "includes/UniqueValuesMapIterator.tpp"
#include <iostream>
#include <string>

class Host {
	public:
		Host(const std::string& name);
		Host(const Host&);
		std::string	name;
};

Host::Host(const std::string& name) : name(name) {}
Host::Host(const Host& copy) : name(copy.name) {}


Host*	insert(std::list<Host>& list, const std::string& name) {
	Host	host(name);
	Host&	ref = *list.insert(list.end(), host);
	return (&ref);
}

int	main(void) {
	
	std::list<Host>	list;
	Host*	host0 = insert(list, "host0");
	Host*	host1 = insert(list, "host1");

	std::map<std::string, Host*>	map;
	map["key0"] = host1;
	map["key1"] = host0;
	map["key2"] = host0;
	map["key3"] = host1;
	map["key4"] = host0;
	map["key4"] = host0;
	// UniqueValuesMapIterator_const<std::string, Host*> it(map.begin());
	// ++it;
	// ++it;
	// ++it;
	for (UniqueValuesMapIterator_const<std::string, Host*> it(map.begin()); it != map.end(); it++) {
		std::cout << it->second->name << std::endl;
	}
}