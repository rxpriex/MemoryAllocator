#include <stddef.h>

#include <stdio.h>

#include "Memory.h"

#ifdef _WIN32
#include <windows.h>
#elif defined(__linux__)
#include <sys/mman.h>
#include <unistd.h>
#endif

#define DEFAULT_MEMORY_SIZE 8
#define DEFAULT_HEAP_SIZE INFINITE
#define DEFAULT_BUFFER_SIZE 128

__int32 getValue(__int64* addr, int defVal) {
	return addr == 0 ? defVal : *((__int32*)(*(__int64*)addr));
}

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

__int64* addrBufferSize = 0;
__int64* addrMemoryBlockSize = 0;
__int64* addrHeapSize = 0;

void _systemcall_free_memory(void* memory) {
#ifdef _WIN32
	VirtualFree(memory, 0, MEM_RELEASE);
#elif defined(__linux__)
	munmap(memory, sizeof(memory));
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

void SVIP(__int64** ptr, __int32 value) {
	__int32* temp;
	if (*ptr == 0) {
		 temp = (__int32*)_systemcall_allocate_memory(sizeof(__int32));
		*temp = value;
		*ptr = _systemcall_allocate_memory(sizeof(__int64));
		**ptr = ((__int64*)(temp));
	}
	else {
		temp = (__int32*)(**ptr);
		*temp = value;
	}
}

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
	if ((countBytes(hfree) + block->size) > getValue(addrBufferSize,DEFAULT_BUFFER_SIZE)) {
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
	if((getAllocatedBytes() + memsize - getFreedBytes()) > getValue(addrHeapSize, DEFAULT_HEAP_SIZE)) return NULL;

	printf("Memory block size:%d\n", getValue(addrMemoryBlockSize, 0));
	memsize = (memsize + getValue(addrMemoryBlockSize,DEFAULT_MEMORY_SIZE) - 1) & ~(getValue(addrMemoryBlockSize, DEFAULT_MEMORY_SIZE) - 1);

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