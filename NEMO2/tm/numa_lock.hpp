#ifndef NUMA_LOCk_HPP
#define NUMA_LOCk_HPP 1

#include <stdint.h>

#define LOCAL 1
#define GLOBAL 2

#define CACHELINE_BYTES2 64

struct pad_word_t2
  {
      volatile uintptr_t val;
      volatile bool successorExists;
      char pad[CACHELINE_BYTES2-sizeof(uintptr_t)-sizeof(bool)];
  };

#define LOCAL_THRESHOLD 10
class Numa_Lock {
	pad_word_t2 global_lock;

	pad_word_t2* local_locks[8];

	int release_type[8];
	int local_acquire_count[8];

	long count;

public:
	Numa_Lock() {
		global_lock.val = 0;
		count = 0;
		for (int i=0; i<4; i++) {
			local_locks[i] = numa_alloc_onnode(sizeof(pad_word_t2), i);
			local_locks[i]->val = 0;
			local_locks[i]->successorExists = false;
			release_type[i] = GLOBAL;
			local_acquire_count[i] = 0;
		}
	}

	__attribute__((always_inline)) inline void lock(int numa) {
		while(1) {
			if (__sync_bool_compare_and_swap(&(local_locks[numa]->val), 0, 1)) {
				if (release_type[numa] == LOCAL) {
					//remove the successor_exist flag
					local_locks[numa]->successorExists = false;
					local_acquire_count[numa]++;
					//printf("locked locally on %d\n", numa);
					//lock is passed from local thread and can proceed to CS
					return;
				} else {
					//We need to acquire the global lock also
					while(!__sync_bool_compare_and_swap(&(global_lock.val), 0, 1)) {
						while(global_lock.val) __asm__ ( "pause;" );
					}
					//printf("locked globally on %d\n", numa);
					return;
				}
			} else { //failed to acquire the local lock, then set successor_exist flag and wait
				local_locks[numa]->successorExists = true;
				while(local_locks[numa]->val) __asm__ ( "pause;" );
			}
		}
	}

	__attribute__((always_inline)) inline void unlock(int numa, bool inc) {
		if (inc) count++;
		if (local_locks[numa]->successorExists && local_acquire_count[numa] < LOCAL_THRESHOLD) {
			release_type[numa] = LOCAL;
			local_locks[numa]->val = 0;
			//printf("released locally on %d\n", numa);
		} else {
			local_acquire_count[numa] = 0;
			global_lock.val = 0;
			release_type[numa] = GLOBAL;
			local_locks[numa]->val = 0;
			//printf("released globally on %d\n", numa);
		}
	}

	__attribute__((always_inline)) inline long get_count() {
		return count;
	}

	__attribute__((always_inline)) inline bool is_locked(int numa) {
		return local_locks[numa]->val;
	}
};

#endif
