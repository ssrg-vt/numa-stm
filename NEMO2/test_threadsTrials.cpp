#include <pthread.h>
#include <signal.h>
#include <pthread.h>

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <stdint.h>
#include <numa.h>
#include <unistd.h>
#include <errno.h>

#define CFENCE  __asm__ volatile ("":::"memory")
#define MFENCE  __asm__ volatile ("mfence":::"memory")

#define CACHELINE_BYTES 64

struct pad_word_t
  {
      volatile int val;
      char pad[CACHELINE_BYTES-sizeof(int)];
  };

volatile pad_word_t owner = {0}, owner_in = {0}, request = {0}, counter = {0};
volatile int flags[100] = {0};
volatile int in[100] = {0};

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

bool ExperimentInProgress = true;
static void catch_SIGALRM(int sig_num)
{
    ExperimentInProgress = false;
}

unsigned long long throughputs[300];

void* th_run(void * args)
{
	int id = 1+((long)args);
	barrier(0);

	if (id == 1) {
		signal(SIGALRM, catch_SIGALRM);
		alarm(1);
	}
	pad_word_t mine;
	int tx_count = 0;
	while(ExperimentInProgress) {
		int granted = 0;
		while(granted == 0) {
			flags[id] = 1;
			if (owner.val == id && request.val == 0) {
				owner_in.val = id;
				__sync_bool_compare_and_swap(&owner_in.val, owner_in.val, id);

//				MFENCE;
//				__sync_synchronize();
//				__sync_bool_compare_and_swap(&flags[id], 1, 1);
				if (owner.val == id) {
					granted = 1;
				} else {
					flags[id] = 0;
					owner_in.val = -id;
				}
			}else if (__sync_bool_compare_and_swap(&(request.val), 0, id)) {
				int old_owner = owner.val;
//				owner.val = id;
				__sync_bool_compare_and_swap(&owner.val, owner.val, id);

//				MFENCE;
//				__sync_synchronize();
//				__sync_bool_compare_and_swap(&flags[id], 1, 1);
				while (owner_in.val != 0 && owner_in.val == old_owner){
					asm volatile ("pause");
				}
				if (owner_in.val != 0) {
					flags[id] = 0;
					while (flags[old_owner]){
						asm volatile ("pause");
					}
					owner_in.val = 0;
					request.val = 0;
				} else {
					granted = 2;
				}
			} else {
				flags[id] = 0;
			}
		}
		in[id] = true;
		counter.val ++;
		tx_count++;
		while (in[1] && in[2]);
		in[id] = false;
		if (granted == 1) {
			owner_in.val = 0;
		} else {
			request.val = 0;
		}
		flags[id] = 0;
	}
    throughputs[id] = tx_count;
	printf("tx_count = %d\n", tx_count);
	return 0;
}

int main(int argc, char* argv[])
{
	signal(SIGINT, signal_callback_handler);

	if (argc < 2) {
		printf("Usage test threads#\n");
		exit(0);
	}

    int th_per_zone = atoi(argv[1]);
	total_threads = th_per_zone? th_per_zone : 1;

	pthread_attr_t thread_attr;
	pthread_attr_init(&thread_attr);

	pthread_t client_th[300];
	int ids = 1;
	for (int i = 1; i<th_per_zone; i++) {
		pthread_create(&client_th[ids-1], &thread_attr, th_run, (void*)ids);
		ids++;
	}

	th_run(0);

	for (int i=0; i<ids-1; i++) {
		pthread_join(client_th[i], NULL);
	}

	unsigned long long totalThroughput = 0;
	for (int i=0; i<=total_threads; i++) {
		totalThroughput += throughputs[i];
	}

	printf("\nCounter should be = %llu\n", totalThroughput);

	printf("counter=%d\n", counter.val);
	return 0;
}
