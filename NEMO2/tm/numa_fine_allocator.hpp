#ifndef NUMA_ALLOC_HPP
#define NUMA_ALLOC_HPP 1

#include "tm_thread.hpp"


extern char* tm_mem_buffer[8];
extern pad_word_t* tm_mem_pos[8];

inline void numa_memory_init() {
	int buffer_size = 1024 * 1024 * 1024; //1GB
	for (int i=0; i<ZONES; i++) {
		tm_mem_buffer[i] = (char*) numa_alloc_onnode(buffer_size, i);
		tm_mem_pos[i] = (pad_word_t*) numa_alloc_onnode(sizeof(pad_word_t), i);
		tm_mem_pos[i]->val = 0;
	}
}

FORCE_INLINE void* numa_malloc(int size, int numa) {
	uintptr_t pos = __sync_fetch_and_add(&tm_mem_pos[numa]->val, size);
	return (void*) (tm_mem_buffer[numa] + pos);
}


#endif //NUMA_ALLOC_HPP
