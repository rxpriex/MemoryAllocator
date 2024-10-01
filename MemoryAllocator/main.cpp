#define C_MEMORY_MANAGEMENT

#include "Memory.h"

#include <iostream>

int main() {
	while (true) {
		_MemorySize(32);
		_BufferSize(128);

		std::string* i = alloc(std::string);

		_free(i);
	}

	return 0;
}