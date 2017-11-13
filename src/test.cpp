#include <string>
#include <iostream>
#include "dict.h"
#include "value.h"

int main() {
	Dict<std::string, zv::Value*> dict;

	zv::Value* p = dict["aaa"];
	if (p == NULL) {
		std::cout << "aaa" << std::endl;
	} else {
		std::cout << "bbb" << std::endl;
	}

	return 0;

}