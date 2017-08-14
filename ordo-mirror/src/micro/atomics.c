#include <sys/mman.h>
#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>

#include "bench.h"

static struct {
   volatile int start;
   union {
	struct {
	    volatile int ready;
	    volatile uint64_t cycle;
	    volatile double tput;
	};
	char pad[CACHE_BYTES];
   } cpu[MAX_CPU] __attribute__((aligned(CACHE_BYTES)));
} *sync_state;

static int ncores;
static volatile unsigned long count = 0;
static unsigned long nops = 100000;
static unsigned long spin_threshold = 0;
char opname[100];

#define spin(num) 				\
	do { 					\
		unsigned long j; 		\
		for (j = 0; j < num; ++j) 	\
			asm volatile("pause"); 	\
	} while (0)

static int64_t get_time_diff_cycles(struct timespec *e,
				    struct timespec *s)
{
	return (e->tv_sec - s->tv_sec) * 1000000000 +
		(e->tv_nsec - s->tv_nsec);
}

static double get_time_diff_sec(struct timespec *e,
				struct timespec *s)
{
	return (double)get_time_diff_cycles(e, s) / 1000000000.0;
}

static void *func_xchg(void *arg)
{
	int tid = (uintptr_t)arg;
	uint64_t i;
	struct timespec s, e;
	double sec;

	setaffinity(tid);

	sync_state->cpu[tid].ready = 1;
	if (tid)
		while (!sync_state->start)
			nop_pause();
	else
		sync_state->start = 1;

	clock_gettime(CLOCK_MONOTONIC, &s);
	for (i = 0; i < nops; ++i) {
		__sync_fetch_and_add(&count, 1);
		spin(spin_threshold);
	}

	clock_gettime(CLOCK_MONOTONIC, &e);
	sync_state->cpu[tid].cycle = get_time_diff_cycles(&e, &s);
	sec = get_time_diff_sec(&e, &s);
	sync_state->cpu[tid].tput = (double)nops / sec;
	return 0;
}

static void *func_cas(void *arg)
{
	uint64_t i;
	struct timespec s, e;
	double sec;
	int tid = (uintptr_t)arg;
	uint64_t time;

	setaffinity(tid);

	sync_state->cpu[tid].ready = 1;
	if (tid)
		while (!sync_state->start)
			nop_pause();
	else
		sync_state->start = 1;

	clock_gettime(CLOCK_MONOTONIC, &s);
	for (i = 0; i < nops; ++i) {
	     retry:
		time = read_tscp();
		if (time > count &&
		    __sync_bool_compare_and_swap(&count, count, time))
			spin(spin_threshold);
		else
			goto retry;
	}

	clock_gettime(CLOCK_MONOTONIC, &e);
	sync_state->cpu[tid].cycle = get_time_diff_cycles(&e, &s);
	sec = get_time_diff_sec(&e, &s);
	sync_state->cpu[tid].tput = (double)nops / sec;
	return 0;
}

static void waitup(void)
{
	uint64_t tot, max;
	int i;
	double avg = 0.0;

	tot = 0;
	max = 0;
	for (i = 0; i < ncores; i++) {
		while (!sync_state->cpu[i].cycle)
			nop_pause();

		tot += sync_state->cpu[i].cycle;
		if (sync_state->cpu[i].cycle > max)
			max = sync_state->cpu[i].cycle;
		avg += sync_state->cpu[i].tput;
	}

	printf("%d cores completed %d %s in %"PRIu64" cycles %lu tput, avg: %.3g\n",
	       ncores, nops, opname, tot, avg/ncores);
}

int main(int ac, char **av)
{
	pthread_t th;
	int i;
	void *(*worker)(void *arg);

	if (ac < 3)
		die("usage: %s num-cores nops cas->1/xchg->2 spin", av[0]);

	setaffinity(0);
	ncores = atoi(av[1]);
	nops = atoi(av[2]);
	if (atoi(av[3]) == 1) {
		worker = &func_cas;
		strcpy(opname, "CAS");
	} else {
		worker = &func_xchg;
		strcpy(opname, "XCHG");
	}
	if (ac > 4)
		spin_threshold = atoi(av[4]);

	sync_state = (void *) mmap(0, sizeof(*sync_state),
				   PROT_READ | PROT_WRITE,
				   MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	if (sync_state == MAP_FAILED)
		edie("mmap sync_state failed");
	memset(sync_state, 0, sizeof(*sync_state));

	for (i = 1; i < ncores; i++) {
		if (pthread_create(&th, NULL, worker, (void *)(intptr_t)i) < 0)
			edie("pthread_create");
		while (!sync_state->cpu[i].ready)
			nop_pause();
	}

	worker((void *)(intptr_t)0);
	waitup();
	return 0;
}
