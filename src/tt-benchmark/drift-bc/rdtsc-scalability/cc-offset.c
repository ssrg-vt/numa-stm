#include <sys/mman.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

#include "bench.h"

static struct {
   volatile int start;
   volatile int phase;
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
static int ncores;
static uint64_t **otable;

#define PHASE_WAIT 1
#define PHASE_RESET 2

pthread_barrier_t pbarrier;

static void *master_worker(void *x)
{
	int i = (uintptr_t)x;
	int k;

	sync_state->cpu[i].ready = 1;
	sync_state->start = 1;

	for (k = 0; k < niter; ++k) {
		sync_state->phase = PHASE_WAIT;
		barrier();
		otable[k][i] = read_tsc();
		barrier();
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
		while (sync_state->phase == 0);
		barrier();
		otable[k][i] = read_tsc();
		barrier();
	}

	sync_state->cpu[i].cycle = 1;
	return 0;
}

static void waitup(void)
{
	int i, j;

	for (i = 0; i < ncores; i++) {
		while (!sync_state->cpu[i].cycle)
			nop_pause();
	}

	for (j = 1; j < ncores; ++j) {
		for (i = 0; i < niter; ++i) {
			printf("core %d: niter %d: diff: %Ld\n",
			       j, i, otable[i][0] - otable[i][j]);
		}
	}
}

int main(int ac, char **av)
{
	pthread_t th;
	int i;

	setaffinity(0);
	if (ac < 2)
		die("usage: %s num-cores niter", av[0]);

	ncores = atoi(av[1]);
	niter = atoi(av[2]);

	if (ncores == 1)
		edie("need more than 1 core");

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
		otable[i] = malloc(sizeof(uint64_t) * ncores);
		if (otable[i] == NULL)
			edie("otable[%d] allocation failed", i);
		memset(otable[i], 0, sizeof(uint64_t) * ncores);
	}

	pthread_barrier_init(&pbarrier, NULL, ncores);
	for (i = 1; i < ncores; i++) {
		if (pthread_create(&th, NULL, worker, (void *)(intptr_t)i) < 0)
			edie("pthread_create");
		while (!sync_state->cpu[i].ready)
			nop_pause();
	}
	master_worker((void *)(intptr_t)0);
	waitup();

	return 0;
}
