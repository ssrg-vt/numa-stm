#include <sys/mman.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

#include "bench.h"

static struct {
   volatile int start;
   volatile int phase;
   volatile int remote_read;
   union {
	struct {
	    volatile int ready;
	    volatile uint64_t cycle;
	    volatile double tput;
	};
	char pad[CACHE_BYTES];
   } cpu[MAX_CPU] __attribute__((aligned(CACHE_BYTES)));
} *sync_state;

static int niter;
int cpu0, cpu1;
static uint64_t **otable;

#define PHASE_WAIT 1
#define PHASE_RESET 2

static void *master_worker(void *x)
{
	int i = (uintptr_t)x;
	int k;
	int count = 0;

	setaffinity(i);
	sync_state->cpu[i].ready = 1;
	sync_state->start = 1;

	while (count++ != 1000)
		rep_nop();
	/*
	 * Make the cacheline exclusive
	 */
	barrier();
	sync_state->phase = 0;
	smp_mb();

	for (k = 0; k < niter; ++k) {
		register uint64_t tmp_time = read_tscp();
		sync_state->phase = PHASE_WAIT;
		smp_wmb();
		//otable[k][0] = read_tscp();
		register uint64_t v = read_tsc();
		smp_mb();
		while (sync_state->phase == PHASE_WAIT);
		/*
		 * Put the master core in the exclusive state
		 */
		sync_state->phase = 0;
		otable[k][2] = tmp_time;
		otable[k][0] = v;
		smp_mb();
	}

	sync_state->cpu[i].cycle = 1;
	return 0;
}

static void *worker(void *x)
{
	int i = (uintptr_t)x;
	int k;

	setaffinity(i);
	sync_state->cpu[i].ready = 1;
	while (!sync_state->start)
		nop_pause();

	for (k = 0; k < niter; ++k) {
		while (sync_state->phase == 0)
			smp_rmb();
		register uint64_t v = read_tsc();
		otable[k][1] = v;
		smp_mb();
		sync_state->phase = 0;
	}

	sync_state->cpu[i].cycle = 1;
	return 0;
}

static void waitup(void)
{
	int i;

	while (!sync_state->cpu[cpu1].cycle)
		nop_pause();

	for (i = 0; i < niter; ++i) {
		printf("%d - %d : %Lu %Lu %Lu\n", cpu0, cpu1,
		       otable[i][0], otable[i][1], otable[i][2]);

	}
}

int main(int ac, char **av)
{
	pthread_t th;
	struct sched_param param;
	int i;

	if (ac < 2)
		die("usage: %s cpu0 cpu1 niter", av[0]);

	cpu0 = atoi(av[1]);
	cpu1 = atoi(av[2]);
	niter = atoi(av[3]);

	sync_state = (void *) mmap(0, sizeof(*sync_state),
				   PROT_READ | PROT_WRITE,
				   MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	if (sync_state == MAP_FAILED)
		edie("mmap sync_state failed");
	memset(sync_state, 0, sizeof(*sync_state));

	otable = malloc(sizeof(uint64_t *) * niter);
	if (otable == NULL)
		edie("otable malloc failed");

	for (i = 0; i < niter; ++i) {
		otable[i] = malloc(sizeof(uint64_t) * 3);
		if (otable[i] == NULL)
			edie("otable[%d] allocation failed", i);
		memset(otable[i], 0, sizeof(uint64_t) * 2);
	}

	param.sched_priority = 1;
	if (sched_setscheduler(getpid(), SCHED_FIFO, &param))
		edie("cannot change policy");

	if (pthread_create(&th, NULL, worker, (void *)(intptr_t)cpu1) < 0)
		edie("pthread_create");
	while (!sync_state->cpu[cpu1].ready)
		nop_pause();
	master_worker((void *)(intptr_t)cpu0);
	waitup();

	return 0;
}
