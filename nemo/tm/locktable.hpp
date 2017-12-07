#ifndef LOCKTABLE_HPP
#define LOCKTABLE_HPP 1

#include <sys/types.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <numaif.h>
#include <numa.h>
#include <stdint.h>
#include <stdio.h>
#include <sched.h>

#define TABLE_SIZE 1048576

#define SHARED_TABLE_KEY 576566400


#define SHARED_MEM_KEY1 571166400
#define SHARED_MEM_KEY2 572266400
#define SHARED_MEM_KEY3 573366400
#define SHARED_MEM_KEY4 574466400
#define SHARED_MEM_KEY5 575566400

#define UNLOCK 0
//first bit set
#define READ_LOCK 1
//first TWO bit set
#define WRITE_LOCK 3

#define READ_LOCK_MULTI_OWNERS     0xFFFF0000000
//READ LOCKED BY TWO
#define READ_LOCK_MULTI_OWNERS_INIT 0xFFFF0000021

//READ LOCKED BY ONE
#define READ_LOCK_MULTI_OWNERS_ONE 0xFFFF0000011

#define LOCK_MASK 0xF

#define UNLOCK_MASK 0xFFFFFFFFFFFFFFF0

#define ONE_MASK 0x10

#define TLC

struct lock_entry {
#ifdef TLC
	uint64_t id;
#endif
	volatile uint64_t lock_owner;
	volatile uint64_t version;
};

//lock_entry * create_lock_table(int numa_zone);
//lock_entry * get_lock_table(int numa_zone);
//void release_memory(void* table);
//void * create_shared_mem(int numa_zone, int size, int shared_key);
//void * get_shared_mem(int numa_zone, int size, int shared_key);

#endif //LOCKTABLE_HPP
