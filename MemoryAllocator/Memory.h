#ifndef _MEMORY_H
#define _MEMORY_H

#define alloc(value) (value*)_allocate(sizeof(value))
#define isAllocated(value) _validateCMP(value)
#define dealloc(value) _free(value)

#ifdef __cplusplus
extern "C" {
#endif

	extern void* _allocate(size_t memsize);

	extern void _free(void* ptr);

	extern int _validateCMP(void* ptr);

	extern void deallocToSize(int value);

	extern size_t getAllocatedBytes();

	extern size_t getFreedBytes();

	extern void SVIP(__int64** ptr, __int32 value);

	extern __int64* addrHeapSize;

	extern __int64* addrBufferSize;

	extern __int64* addrMemoryBlockSize;

	#define _HeapSize(value) SVIP(&addrHeapSize,value)
	#define _MemorySize(value) SVIP(&addrMemoryBlockSize,value)
	#define _BufferSize(value) SVIP(&addrBufferSize,value)

#ifdef __cplusplus
}

#ifdef C_MEMORY_MANAGEMENT

void* operator new(size_t size) {
	return _allocate(size);
}

void operator delete(void* ptr) {
	_free(ptr);
}

#endif
#endif
#endif _MEMORY_H