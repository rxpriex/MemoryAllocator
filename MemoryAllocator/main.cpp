#define C_MEMORY_MANAGEMENT

#include "Memory.h"

#include <iostream>

int main() {
	while (true) {
		for (int i = 0; i < 40; i++) {
			std::string* str = alloc(std::string);
		}

		std::cout << getAllocatedBytes() << std::endl;
		deallocToSize(40);
		std::cout << getAllocatedBytes() << std::endl;
	}

	return 0;
}