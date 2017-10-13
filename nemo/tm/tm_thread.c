#include "tm_thread.hpp"
#include "numa_lock.hpp"
#include <pthread.h>
#include <signal.h>
#include <pthread.h>

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

//pad_word_t* timestamp;
//pad_word_t last_init     = {0};
//pad_word_t last_complete = {0};

__thread Tx_Context* Self;

pad_word_t active_tx = {0};
//pad_word_t tm_super_glock = {0};

//volatile int tm_thread_count = 8;

//char* tm_mem_buffer;
//pad_word_t tm_mem_pos = {0};
//NOT part of our library but used in STAMP

long int    FALSE = 0,
    TRUE  = 1;

ts_vector* ts_vectors[ZONES];
pad_word_t* ts_loc[ZONES];
lock_entry* lock_table[ZONES];
pad_msg_t* comm_channel[ZONES];
queue* zone_queues[ZONES];

Numa_Lock numa_lock;

#ifdef DEBUG

int logTM[LOG_SIZE];
int logiTM = 0;

#endif

//TODO get zone thread count also
void* server_run(void * args)
{
    int numa_zone = (long) args;


    bitmask* mask = numa_allocate_cpumask();

    //get # of cpu per node
    numa_node_to_cpus(0, mask);
    int cpu_per_node = 0;
    while(numa_bitmask_isbitset(mask, cpu_per_node)) {
    	cpu_per_node++;
    }

    //printf("cpus per node = %d\n", cpu_per_node);

    numa_bitmask_clearall(mask);
    numa_bitmask_setbit(mask, numa_zone * cpu_per_node);
    if (numa_sched_setaffinity(0, mask)) {
    	perror("numa_sched_setaffinity");
		exit(-1);
    }
    numa_free_cpumask(mask);
    int curcpu = sched_getcpu();
    	    printf("Server on %d cpu set to %d\n", numa_zone, curcpu);

    while (true) {
    	for (int i=0; i<50; i++) {
    		if (comm_channel[numa_zone][i].ready) {
    			comm_channel[numa_zone][i].ready = 0;
    			comm_channel[numa_zone][i].result = tm_srv_commit(comm_channel[numa_zone][i].tx);
    		}
    	}
    }
}

//int SUPER_GL_LIMIT = 1;
//int SUPER_GL_LIMIT2 = 1;
