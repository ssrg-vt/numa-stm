#include "tm/tm_thread.hpp"
#include <pthread.h>
#include <signal.h>
#include <pthread.h>

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

#include "tm/rand_r_32.h"
#include <errno.h>

#include "tm/numa_fine_allocator.hpp"
#include "bench/Hash.hpp"

#define LIST_SIZE 256
#define LOOKUP 40
#define INSERT 70
#define N_BUCKETS 256
HashTable* SET;

int total_threads;
/**
 *  Support a few lightweight barriers
 */
void
barrier(uint32_t which)
{
    static volatile uint32_t barriers[16] = {0};
    CFENCE;
    __sync_fetch_and_add(&barriers[which], 1);
    while (barriers[which] != total_threads) { }
    CFENCE;
}

int myid;
void
signal_callback_handler(int signum)
{
   printlogTM(myid);
   // Terminate program
   exit(signum);
}

bool ExperimentInProgress = true;
static void catch_SIGALRM(int sig_num)
{
    ExperimentInProgress = false;
}

unsigned long long throughputs[64];

void* th_run(void * args)
{

	int id = (long) args >> 4;
    int numa_zone = (long) args & 0xF;

    //printf("my id %d and zone %d\n", id, numa_zone);

	//assume symmetric numa zones
	//printf("numa zones count = %d\n", numa_num_configured_nodes());
	//pin the thread to the numa zone
	bitmask* mask = numa_allocate_cpumask();

	//get # of cpu per node
	numa_node_to_cpus(0, mask);
	int cpu_per_node = 0;
	while(numa_bitmask_isbitset(mask, cpu_per_node)) {
		cpu_per_node++;
	}

	//printf("cpus per node = %d\n", cpu_per_node);

	numa_bitmask_clearall(mask);
	numa_bitmask_setbit(mask, numa_zone * cpu_per_node + (id % cpu_per_node));
	if (numa_sched_setaffinity(0, mask)) {
		perror("numa_sched_setaffinity");
		exit(-1);
	}
	numa_free_cpumask(mask);

//	    int curcpu = sched_getcpu();
//	    printf("%d:%d cpu set to %d\n", id, numa_zone, curcpu);

	  thread_init(id, numa_zone);
	  //printf("Thread waiting\n");

	  barrier(0);

	unsigned int seed = id;

	if (id == 0) {
		signal(SIGALRM, catch_SIGALRM);
		alarm(1);
	}

	unsigned long long time = get_real_time();
	int tx_count = 0;
	while(ExperimentInProgress) {
		tx_count++;
	    uint32_t val = rand_r_32(&seed) % (N_BUCKETS);
	    uint32_t act = rand_r_32(&seed) % 100;
	    int set_numa = numa_zone;
	    if (tx_count % 10 == 0){
	    	set_numa = (numa_zone+1) % 8;
	    }

	    val += (set_numa * (N_BUCKETS));

	    if (act < LOOKUP) {
	        TM_BEGIN {
	            SET->lookup(val TM_PARAM);
	        } TM_END;
	    }
	    else if (act < INSERT) {
	        TM_BEGIN {
	            SET->insert(val TM_PARAM);
	        } TM_END;
	    }
	    else {
	        TM_BEGIN {
	            SET->remove(val TM_PARAM);
	        } TM_END;
	    }
	}
	time = get_real_time() - time;

	printlogTM(myid);
    //curcpu = sched_getcpu();
    //printf("cpu set to %d\n", curcpu);

	//printf("%d: Total Time = %llu\n", id, time);
	//printf("%d: Throughput = %llu\n", id, (1000000000LL * tx_count) / (time));
    throughputs[id] = (1000000000LL * tx_count) / (time);
	TM_TX_VAR
	printf("%d: commits = %d, aborts = %d, my zone %d, out of zone = %d\n", id, tx->commits, tx->aborts, tx->numa_zone, tx->internuma);

	return 0;
}

int main(int argc, char* argv[])
{
	signal(SIGINT, signal_callback_handler);

	tm_sys_init();

	numa_memory_init();

	if (argc < 2) {
		printf("Usage test threads_per_zone#\n");
		exit(0);
	}

    int th_per_zone = atoi(argv[1]);
	total_threads = th_per_zone? th_per_zone * 8 : 1;


	SET = new HashTable();
	// warm up the datastructure
	for (uint32_t w = 0; w < LIST_SIZE; w+=2)
		SET->insert_NoTM(w);

	pthread_attr_t thread_attr;
	pthread_attr_init(&thread_attr);

	pthread_t client_th[64];
	int ids = 1;
	for (int j=0; j < 8; j++) {
		for (int i = 0; i<th_per_zone; i++) {
			if (j==0 && i==0) continue;
			long encodedInfo = (ids << 4) | j;
			pthread_create(&client_th[ids-1], &thread_attr, th_run, (void*)encodedInfo);
			ids++;
		}
	}

	th_run(0);

	for (int i=0; i<ids-1; i++) {
		pthread_join(client_th[i], NULL);
	}

	unsigned long long totalThroughput = 0;
	for (int i=0; i<total_threads; i++) {
		totalThroughput += throughputs[i];
	}

	printf("\nThroughput = %llu\n", totalThroughput);


	printf("is sane? %d\n", SET->isSane());

	return 0;
}
