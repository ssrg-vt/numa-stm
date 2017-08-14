/*
* Create threads, mmap non-overlapping address range, reference it, and unmap it.
*/

#include <sys/mman.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

#include "bench.h"
#include "interval_tree.h"

#ifndef __KERNEL__
#define rwsem_t 	pthread_rwlock_t
#define deinit_rwsem(_l) pthread_rwlock_destroy(_l)
#define down_write(_l) 	pthread_rwlock_wrlock(_l)
#define up_write(_l) 	pthread_rwlock_unlock(_l)
#define down_read(_l) 	pthread_rwlock_rdlock(_l)
#define up_read(_l) 	pthread_rwlock_unlock(_l)
#endif

#define BLOCK_SIZE 	(1ULL << 40)

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

static int niter;
static int ncores;
static int rperc = 10;

struct rb_root tree = RB_ROOT;
rwsem_t rb_sem = PTHREAD_RWLOCK_INITIALIZER;

static uint64_t search_tree(uint64_t start, uint64_t last,
			    struct rb_root *root)
{
	struct interval_tree_node *node;
	uint64_t results = 0;
	for (node = interval_tree_iter_first(root, start, last); node;
	     node = interval_tree_iter_next(node, start, last))
		++results;
	return results;
}

static void insert_tree(struct rb_root *root, struct interval_tree_node *node,
			uint64_t *start, int num_pages)
{
	node->start = *start;
	node->last = *start + PAGE_SIZE * num_pages - 1;
	interval_tree_insert(node, root);
	*start = node->last;
}

static void *worker(void *x)
{
	int i = (uintptr_t)x;
	uint64_t s;
	int k;
	uint64_t bstart = i * BLOCK_SIZE;
	uint64_t bmid = bstart + BLOCK_SIZE/2;
	uint64_t start = bstart;
	uint64_t mid = bmid;
	uint64_t t;

	struct interval_tree_node *nodes =
		malloc(sizeof(struct interval_tree_node) * niter);
	if (!nodes) {
		edie("cannot allocate nodes");
	}

	setaffinity(i);
	sync_state->cpu[i].ready = 1;
	if (i)
		while (!sync_state->start)
			nop_pause();
	else
		sync_state->start = 1;

	s = read_tsc();
	t = usec();

	for (k = 1; k <= niter; k++) {
		if (k%rperc  == 0) {
			down_read(&rb_sem);
			if (k%20)
				search_tree(start + PAGE_SIZE * (k % 10),
					    PAGE_SIZE, &tree);
			else
				search_tree(mid + PAGE_SIZE * (k % 20),
					    PAGE_SIZE, &tree);
			up_read(&rb_sem);
		} else {
			down_write(&rb_sem);
			if (k % 2)
				insert_tree(&tree, &nodes[k], &bmid, k % 10);
			else
				insert_tree(&tree, &nodes[k], &bstart, k % 200);
			up_write(&rb_sem);
		}
	}

	sync_state->cpu[i].cycle = read_tsc() - s;
	double sec = (usec() - t) / 1000000000.0;
	sync_state->cpu[i].tput = (double) niter / sec;
	//free(nodes);
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

	printf("%d cores completed %d spinlock_update in %"PRIu64" cycles %lu tput %.3g\n",
	       ncores, niter, avg, avg/ncores);
}

int main(int ac, char **av)
{
	pthread_t th;
	int i;

	setaffinity(0);
	if (ac < 3)
		die("usage: %s num-cores niter (max 1M)", av[0]);

	ncores = atoi(av[1]);
	niter = atoi(av[2]);
	if (ac == 4)
		rperc = atoi(av[3]);

	rperc = (rperc * niter / 100);

	if (niter > 1000000) {
		printf("only 1M is allowed, resetting it\n");
		niter = 1000000;
	}

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
