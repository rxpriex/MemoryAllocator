#include <windows.h>
#include <stddef.h>

#include "Memory.h"

#define MEMORY_BLOCK_SIZE 2
#define MAX_FREED_BYTES 128

typedef struct MemoryBlock {
	size_t size;
	struct MemoryBlock* next;
} MemoryBlock;

static MemoryBlock* hfree;
static MemoryBlock* hheap;

static void freeMemory(void* ptr) {
	VirtualFree(ptr, 0, MEM_RELEASE);
}

static size_t countBytes(MemoryBlock* b) {
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

static void removeFromHeap(MemoryBlock* block) {
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

void cfree(void* ptr) {
	if (ptr == NULL) return;
	MemoryBlock* block = (MemoryBlock*)ptr - 1;
	removeFromHeap(block);
	if ((countBytes(hfree) + block->size) > MAX_FREED_BYTES) {
		freeMemory(ptr);
		return;
	}
	block->next = hfree;
	hfree = block;
	memset((char*)(block + 1), 0, block->size);
}

static void processOversizedMemoryBlock(size_t size, MemoryBlock* memory) {
	if (size < memory->size) {
		size_t remainingSize = memory->size - size;
		MemoryBlock* newBlock = (MemoryBlock*)((char*)(memory + 1) + size);
		newBlock->size = remainingSize;
		newBlock->next = NULL;
		memory->size = size;
		memset((char*)(newBlock + 1), 0, remainingSize);
		cfree((void*)(newBlock + 1));
	}
}

void* cRalloc(size_t memsize) {
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

	void* ptr = VirtualAlloc(NULL, memsize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	if (ptr == NULL) return NULL;
	MemoryBlock* allocatedMemory = (MemoryBlock*)ptr;
	allocatedMemory->size = memsize;
	allocatedMemory->next = hheap;
	hheap = allocatedMemory;

	return (void*)(allocatedMemory + 1);
}