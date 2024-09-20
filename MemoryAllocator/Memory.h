#ifndef _MEMORY_H
#define _MEMORY_H

#ifdef __cplusplus
extern "C" {
#endif

	void* _allocate(size_t memsize);

	void _free(void* ptr);

	size_t getAllocatedBytes();

	size_t getFreedBytes();

#ifdef __cplusplus
}
#endif
#endif _MEMORY_H