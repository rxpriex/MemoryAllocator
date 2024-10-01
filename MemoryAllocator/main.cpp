#define C_MEMORY_MANAGEMENT

#include "Memory.h"

#include <iostream>

int main() {

	_MemorySize(36);
	_BufferSize(128);

	int* i = new int();
	*i = 12;

	std::cout << i << "/" << *i << "/" << getAllocatedBytes() << std::endl;

	return 0;
}