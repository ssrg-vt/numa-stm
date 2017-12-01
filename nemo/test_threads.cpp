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

tm_obj<short>* accountsAll;
#define ACCOUT_NUM 1048576

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

void
signal_callback_handler(int signum)
{
   // Terminate program
   exit(signum);
}

volatile bool ExperimentInProgress = true;
static void catch_SIGALRM(int sig_num)
{
    ExperimentInProgress = false;
}

unsigned long long throughputs[300];

void* th_run(void * args)
{

	int id = ((long)args >> 4) & 0xFF;
    int numa_zone = (long) args & 0xF;
    int index = (long) args >> 20;
    printf("my id %d and zone %d (index %d)\n", id, numa_zone, index);
    	
    	tm_obj<short>* accounts = accountsAll;

    	tm_obj<short>* accounts2 = accountsAll;

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
	    numa_bitmask_setbit(mask, numa_zone * cpu_per_node + ((id

#ifdef RTC
	    		+1
#endif
				)% cpu_per_node)
	    );
	    if (numa_sched_setaffinity(0, mask)) {
	    	perror("numa_sched_setaffinity");
			exit(-1);
	    }
	    numa_free_cpumask(mask);

	    int curcpu = sched_getcpu();
	    printf("%d:%d cpu set to %d\n", id, numa_zone, curcpu);

		  thread_init(id, numa_zone, index);
		  //printf("Thread waiting\n");

		  barrier(0);

		  thread_init(id, numa_zone, index);

		  barrier(1);
	unsigned int seed = id;

	if (id == 0) {
		signal(SIGALRM, catch_SIGALRM);
		alarm(1);
	}

	unsigned long long time = get_real_time();
//	uint64_t max = 0;
//	uint64_t total = 0;
	int tx_count = 0;
	while(ExperimentInProgress) {
		//printf("it #%d\n", i);
		//int retries = 0;
		int acc1[1000];

		int acc2[1000];
		bool once = true;
#define CHUNKS 50
		int arrStart = (rand_r_32(&seed) % (ACCOUT_NUM/CHUNKS)) * (CHUNKS);
		for (int j=0; j< 10; j++) {
//			if (tx_count % 50 == 0) {
//				acc1[j] = (ACCOUT_NUM/total_threads)*id + j;//rand_r_32(&seed) % (ACCOUT_NUM/total_threads);
//				acc2[j] = (ACCOUT_NUM/total_threads)*id + j;//rand_r_32(&seed) % (ACCOUT_NUM/total_threads);
//				acc1[j] = (ACCOUT_NUM/total_threads)*id + rand_r_32(&seed) % (ACCOUT_NUM/total_threads);
//				acc2[j] = (ACCOUT_NUM/total_threads)*id + rand_r_32(&seed) % (ACCOUT_NUM/total_threads);
//				acc1[j] = rand_r_32(&seed) % (ACCOUT_NUM);
//				acc2[j] = rand_r_32(&seed) % (ACCOUT_NUM);
				acc1[j] = arrStart + rand_r_32(&seed) % (CHUNKS);
				acc2[j] = arrStart + rand_r_32(&seed) % (CHUNKS);
//			} else {
//				acc1[j] = rand_r_32(&seed) % (ACCOUT_NUM/8 -1);
//				acc2[j] = rand_r_32(&seed) % (ACCOUT_NUM/8 -1);
//				if (tx_count % 50 == 0 && once) {
//                    acc1[j] = acc1[j] * 8 + numa_zone;
//                    acc2[j] = acc2[j] * 8 + numa_zone + 1;
////					acc1[j] = acc1[j] * 8 + (rand_r_32(&seed) % 8);
////					acc2[j] = acc2[j] * 8 + (rand_r_32(&seed) % 8);
//				} else {
//					acc1[j] = acc1[j] * 8 + numa_zone;
//					acc2[j] = acc2[j] * 8 + numa_zone;
//				}

//				if (acc1[j] % 8 == 0 || acc2[j] % 8 == 0) {
//					printf("acc1 %d, acc2 %d, zone %d\n", acc1[j], acc2[j], numa_zone);
//				}
//			}
			//if (acc1[j] == acc2[j])
				//acc2[j]= (acc1[j] + 2) % ACCOUT_NUM;
		}

		tx_count++;
//		uint64_t start = tick();
		TM_BEGIN2
//		if (!ExperimentInProgress) {
//			printf("Failed\n");
//			return -1;
//		}
		//	retries++;
			//printf("ret = %d\n", retries);
			//if (retries >5) goto endtx;
			for (int j=0; j< 10; j++) {
//				if (tx_count % 10 == 0 && once){
//					TM_WRITE_Z(accounts2[acc1[j]], TM_READ_Z(accounts2[acc1[j]], (numa_zone +1)%ZONES) + 50, (numa_zone +1)%ZONES);
//					TM_WRITE_Z(accounts2[acc2[j]], TM_READ_Z(accounts2[acc2[j]], (numa_zone +1)%ZONES) - 50, (numa_zone +1)%ZONES);
//					once = false;
//				} else {
					TM_WRITE_Z(accounts[acc1[j]],(short)(TM_READ_Z(accounts[acc1[j]],0) + 50),0);
					TM_WRITE_Z(accounts[acc2[j]],(short)(TM_READ_Z(accounts[acc2[j]],0) - 50),0);
//				}
			}
//			for (int k=0; k < 100000; k++) {
//				nop();
//			}
		TM_END2
//		uint64_t tx_t = tick() - start;
//		if (tx_t > max) max=tx_t;
//		total +=tx_t;
	}
	//endtx:
	//printf("jj= %d, x= %d\n", jj, x);

//  pthread_t thread1, thread2, thread3, thread4;
//  pthread_attr_t thread_attr;
//  pthread_attr_init(&thread_attr);

//	pthread_create(&thread1, &thread_attr, tx_fn, (void*)45678);
//	pthread_create(&thread2, &thread_attr, tx_fn, (void*)4968147);
//	pthread_create(&thread3, &thread_attr, tx_fn, (void*)49147);
//	pthread_create(&thread4, &thread_attr, tx_fn, (void*)4967);

//	barrier =0;
//
//	pthread_join(thread1, NULL);
//	pthread_join(thread2, NULL);
//	pthread_join(thread3, NULL);
//	pthread_join(thread4, NULL);
	time = get_real_time() - time;

//	printlogTM(myid);
    //curcpu = sched_getcpu();
    //printf("cpu set to %d\n", curcpu);

	//printf("%d: Total Time = %llu\n", id, time);
	//printf("%d: Throughput = %llu\n", id, (1000000000LL * tx_count) / (time));
    throughputs[id] = (1000000000LL * tx_count) / (time);
	TM_TX_VAR
	printf("%d: commits = %d, aborts = %d, my zone %d, out of zone = %d\n", id, tx->commits, tx->aborts, tx->numa_zone, tx->internuma);

//	long sum = 0;
//	for (int i=0; i<ACCOUT_NUM; i++) {
//		//printf("%d %d %d | ", accounts[i].id, accounts[i].ver, accounts[i].val);
//		sum += accounts[i];
//	}
//	printf("\nsum = %d\n", sum);
//	printf("\nmax tx time = %llu, avg = %llu\n", max, total/(unsigned long long)tx->commits);
	return 0;
}

int main(int argc, char* argv[])
{
	signal(SIGINT, signal_callback_handler);

	tm_sys_init();

	if (argc < 2) {
		printf("Usage test threads#\n");
		exit(0);
	}

//	  for (int i=0; i<8;i++) {
//		  create_lock_table(i);
//	  }

    int th_per_zone = atoi(argv[1]);
	total_threads = th_per_zone? th_per_zone : 1;

//	for (int j=0; j < ZONES; j++) {
		accountsAll = (tm_obj<short>*) numa_alloc_onnode(sizeof(tm_obj<short>) * ACCOUT_NUM, 0);//malloc(sizeof(long) * ACCOUT_NUM);// create_shared_mem(j, sizeof(long) * ACCOUT_NUM, SHARED_MEM_KEY5);//createSharedMem(j);
//	}

	long initSum = 0, initSum2 = 0;
//	for (int j=0; j<ZONES; j++)
		for (int i=0; i<ACCOUT_NUM; i++) {
			accountsAll[i].val = 100;
			accountsAll[i].lock = 0;
			accountsAll[i].ver = 0;
			accountsAll[i].lock_p = &accountsAll[i].lock;
			initSum += 100.0;
		}
		for (int i=0; i<ACCOUT_NUM; i++) {
			initSum2 += accountsAll[i].val;
		}
	printf("init sum = %d, %d, %d\n", initSum, initSum2, ACCOUT_NUM*100);

    bitmask* mask = numa_allocate_cpumask();

    //get # of cpu per node
    numa_node_to_cpus(0, mask);
    int cpu_per_node = 0;
    while(numa_bitmask_isbitset(mask, cpu_per_node)) {
    	cpu_per_node++;
    }
	printf("cpus per node = %d\n", cpu_per_node);


//	thread_init(1, 0, 0);
//
//	tm_obj<float> first;
//	first.val = 100.5;
//	first.lock = 0;
//	first.lock_p = &first.lock;
//
//	tm_obj<char> first2;
//	first2.val = 50;
//	first2.lock = 0;
//	first2.lock_p = &first2.lock;
//
//	tm_obj<short> first3;
//	first3.val = 100;
//	first3.lock = 0;
//	first3.lock_p = &first3.lock;
//
//	float r, r2;
//	char c1, c2;
//	short s1, s2;
//	TM_BEGIN2
//		r = TM_READ_Z(first,0);
//
//		TM_WRITE_Z(first, r + 50,0);
//
//		c1 = TM_READ_Z(first2,0);
//
//		TM_WRITE_Z(first2, (char)(c1 + 50),0);
//
//		s1 = TM_READ_Z(first3,0);
//
//		TM_WRITE_Z(first3, (short)(s1 + 50),0);
//	TM_END2
//
//	TM_BEGIN2
//		r2 = TM_READ_Z(first,0);
//		c2 = TM_READ_Z(first2,0);
//		s2 = TM_READ_Z(first3,0);
//	TM_END2
//
//	printf("floats = %f, %f\n", r, r2);
//	printf("chars = %d, %d\n", c1, c2);
//	printf("shorts = %d, %d\n", s1, s2);

	pthread_attr_t thread_attr;
	pthread_attr_init(&thread_attr);

	pthread_t client_th[300];
	int ids = 1;
//	for (int j=0; j < ZONES; j++) {
		for (int i = 1; i<th_per_zone; i++) {
			long encodedInfo = (i << 20) | (ids << 4) | (i/cpu_per_node);
			pthread_create(&client_th[ids-1], &thread_attr, th_run, (void*)encodedInfo);
			ids++;
		}
//	}

	th_run(0);
	//printf("jj= %d, x= %d\n", jj, x);

//  pthread_t thread1, thread2, thread3, thread4;
//  pthread_attr_t thread_attr;
//  pthread_attr_init(&thread_attr);

//	pthread_create(&thread1, &thread_attr, tx_fn, (void*)45678);
//	pthread_create(&thread2, &thread_attr, tx_fn, (void*)4968147);
//	pthread_create(&thread3, &thread_attr, tx_fn, (void*)49147);
//	pthread_create(&thread4, &thread_attr, tx_fn, (void*)4967);

//	barrier =0;
//
//	pthread_join(thread1, NULL);
//	pthread_join(thread2, NULL);
//	pthread_join(thread3, NULL);
//	pthread_join(thread4, NULL);

	for (int i=0; i<ids-1; i++) {
		pthread_join(client_th[i], NULL);
	}

	unsigned long long totalThroughput = 0;
	for (int i=0; i<total_threads; i++) {
		totalThroughput += throughputs[i];
	}

	printf("\nThroughput = %llu\n", totalThroughput);

	long sum = 0;
	int c=0;
//	for (int j=0; j<ZONES; j++)
		for (int i=0; i<ACCOUT_NUM; i++) {
			//printf("%d %d %d | ", accounts[i].id, accounts[i].ver, accounts[i].val);
			sum += accountsAll[i].val;
			if (accountsAll[i].val != 100) {
				c++;
			}
		}

	for (int i=0; i<ACCOUT_NUM; i++) {
		if (*accountsAll[i].lock_p != 0)
			printf("||%x\t%x\n", accountsAll[i].lock_p, *accountsAll[i].lock_p);
//			printf("||%x   %x   %d\n", accountsAll[0][i].lock_p, &accountsAll[0][i].lock, *accountsAll[0][i].lock_p);
//		if (accountsAll[0][i].lock != 0)
//			printf("**%x   %x   %d\n", accountsAll[0][i].lock_p, &accountsAll[0][i].lock, *accountsAll[0][i].lock_p);
	}
	printf("\nsum = %d, matched = %d, changed %llu\n", sum, sum == initSum, c);

	return 0;
}
