#include <stddef.h>

#include "Memory.h"

#ifdef _WIN32
#include <windows.h>
#elif defined(__linux__)
#include <sys/mman.h>
#include <unistd.h>
#endif

#define MEMORY_BLOCK_SIZE 2
#define MAX_FREED_BYTES 128

void* _systemcall_allocate_memory(size_t size) {
#ifdef _WIN32
	return VirtualAlloc(NULL, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
#elif defined(__linux__)
	return mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
#else
	printf("Failed: Unsupported OS");
	exit(1);
#endif
}

void _systemcall_free_memory(void* memory) {
#ifdef _WIN32
	VirtualFree(memory, 0, MEM_RELEASE);
#elif defined(__linux__)
	munmap(memory, size);
#else
	printf("Failed: Unsupported OS");
	exit(1);
#endif
}

typedef struct MemoryBlock {
	size_t size;
	struct MemoryBlock* next;
} MemoryBlock;

MemoryBlock* hfree;
MemoryBlock* hheap;

size_t countBytes(MemoryBlock* b) {
	size_t total_size = 0;
	MemoryBlock* curr = b;
	while (curr != NULL) {
		total_size += curr->size;
		curr = curr->next;
	}
	return total_size;
}

size_t getAllocatedBytes() { return countBytes(hheap); }

size_t getFreedBytes() { return countBytes(hfree); }

void removeFromHeap(MemoryBlock* block) {
	MemoryBlock* curr = hheap;
	MemoryBlock* prev = NULL;
	while (curr != NULL) {
		if (curr == block) {
			if (prev == NULL) {
				hheap = NULL;
			}
			else {
				prev->next = block->next;
			}
			return;
		}
		prev = curr;
		curr = curr->next;
	}
}

void _free(void* ptr) {
	if (ptr == NULL) return;
	MemoryBlock* block = (MemoryBlock*)ptr - 1;
	removeFromHeap(block);
	if ((countBytes(hfree) + block->size) > MAX_FREED_BYTES) {
		_systemcall_free_memory(ptr);
		return;
	}
	block->next = hfree;
	hfree = block;
	memset((char*)(block + 1), 0, block->size);
}

void processOversizedMemoryBlock(size_t size, MemoryBlock* memory) {
	if (size < memory->size) {
		size_t remainingSize = memory->size - size;
		MemoryBlock* newBlock = (MemoryBlock*)((char*)(memory + 1) + size);
		newBlock->size = remainingSize;
		newBlock->next = NULL;
		memory->size = size;
		memset((char*)(newBlock + 1), 0, remainingSize);
		_free((void*)(newBlock + 1));
	}
}

void* _allocate(size_t memsize) {
	if (memsize <= 0) return NULL;

	memsize = (memsize + MEMORY_BLOCK_SIZE - 1) & ~(MEMORY_BLOCK_SIZE - 1);

	MemoryBlock* prev = NULL;
	MemoryBlock* curr = hfree;
	while (curr != NULL) {
		if (curr->size >= memsize) {
			if (prev == NULL) {
				hfree = curr->next;
			}
			else {
				prev->next = curr->next;
			}
			if (curr->size > memsize) {
				processOversizedMemoryBlock(memsize, curr);
			}

			curr->next = hheap;
			hheap = curr;
			return (void*)(curr + 1);
		}
		prev = curr;
		curr = curr->next;
	}

	void* ptr = _systemcall_allocate_memory(memsize);
	if (ptr == NULL) return NULL;
	MemoryBlock* allocatedMemory = (MemoryBlock*)ptr;
	allocatedMemory->size = memsize;
	allocatedMemory->next = hheap;
	hheap = allocatedMemory;

	return (void*)(allocatedMemory + 1);
}