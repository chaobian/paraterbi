

#ifndef __UTILITY_HPP__
#define __UTILITY_HPP__
#include <string>
#include <iostream>

template<typename container_T>
void showAll(const std::string& name, const container_T& v) {
	std::cout << name << "(" << v.size() << "):\n";
	for (auto& e : v) {
		std::cout << e << ", ";
	}
	std::cout << std::endl;
}

#endif
