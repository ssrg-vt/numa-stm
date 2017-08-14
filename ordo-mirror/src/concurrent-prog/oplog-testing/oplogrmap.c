/*
* Create threads, mmap non-overlapping address range, reference it, and unmap it.
*/

#include <sys/mman.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

#include "bench.h"
#include "interval_tree.h"
#include "list.h"

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
#define CLOCK_DIFF 376

struct rmap_log {
	struct mcslock_t lock;
	struct list_head head;
};

struct rmap_obj {
	struct interval_tree_node node;
	uint64_t time;
	struct list_head list_node;
};

struct mcsqnode_t qnodes[MAX_CPU];
struct rb_root tree = RB_ROOT;
rwsem_t rb_sem = PTHREAD_RWLOCK_INITIALIZER;
struct rmap_log rlog[MAX_CPU];

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
static inline void __add_list(struct list_head *h1, struct list_head *h)
{
	struct list_head *pos, *tmp;
	list_for_each_safe(pos, tmp, h1) {
		list_move_tail(pos, h);
	}
}

static void merge_two_lists(struct list_head *h1, struct list_head *h2)
{
	struct rmap_obj *r1, *r2;
	struct list_head dummy;
	INIT_LIST_HEAD(&dummy);
	while (!list_empty(h1) && !list_empty(h2)) {
		r1 = list_entry(h1->next, struct rmap_obj, list_node);
		r2 = list_entry(h2->next, struct rmap_obj, list_node);
		if (r1->time <= r2->time) {
			list_move_tail(h1->next, &dummy);
			list_move_tail(h2->next, &dummy);
		} else {
			list_move_tail(h2->next, &dummy);
			list_move_tail(h1->next, &dummy);
		}
	}
	if (!list_empty(h1))
		__add_list(h1, &dummy);
	if (!list_empty(h2))
		__add_list(h2, &dummy);
	h1 = &dummy;
}

/* TODO: Use lockless queue will remove the atomic operation cost
 * at least from the unlock phase but won't be there in the kernel
 * space
 */
static void merge_two_logs(struct rmap_log *l1, struct rmap_log *l2,
			   struct list_head *head)
{
	struct rmap_obj *r1, *r2;
	struct list_head *h1 = &l1->head, *h2 = &l2->head;
	struct mcsqnode_t qnode;

	while (!list_empty(h1) && !list_empty(h2)) {
		r1 = list_entry(h1->next, struct rmap_obj, list_node);
		r2 = list_entry(h2->next, struct rmap_obj, list_node);
		if (r1->time<= r2->time) {
			mcslock_lock(&l1->lock, &qnode);
			list_move_tail(h1->next, head);
			mcslock_unlock(&l1->lock, &qnode);

			mcslock_lock(&l2->lock, &qnode);
			list_move_tail(h2->next, head);
			mcslock_unlock(&l2->lock, &qnode);
		} else {
			mcslock_lock(&l2->lock, &qnode);
			list_move_tail(h2->next, head);
			mcslock_unlock(&l2->lock, &qnode);

			mcslock_lock(&l1->lock, &qnode);
			list_move_tail(h1->next, head);
			mcslock_unlock(&l1->lock, &qnode);
		}
	}
	mcslock_lock(&l1->lock, &qnode);
	if (!list_empty(h1))
		__add_list(h1, head);
	mcslock_unlock(&l1->lock, &qnode);

	mcslock_lock(&l2->lock, &qnode);
	if (!list_empty(h2))
		__add_list(h2, head);
	mcslock_unlock(&l2->lock, &qnode);
}

static inline void build_tree(struct rb_root *root, struct rmap_log *log)
{
	int i, j;
	int phase = 0;
	struct rmap_obj *pos;
	int k = ncores / 2 + ncores % 2;
	struct list_head head[k];

	for (i = 0; i < k; ++i)
		INIT_LIST_HEAD(&head[i]);

	i = 0;
	j = ncores - 1;
	while (j != 0) {
		while(i < j) {
			if (phase == 0) {
				merge_two_logs(&log[i], &log[j], &head[i]);
				i++;
				j--;
			} else
				merge_two_lists(&head[i++], &head[j--]);
		}
		i = 0;
		phase = 1;
	}

	list_for_each_entry(pos, &head[0], list_node) {
		interval_tree_insert(&pos->node, root);
	}
}

static inline uint64_t get_time(const uint64_t t)
{
	static int new_time;
	int diff = CLOCK_DIFF - (t % CLOCK_DIFF);
	while ((new_time = read_tsc() - t)  < diff)
		nop_pause();
	return new_time / CLOCK_DIFF;
}

static void insert_tree(struct rb_root *root, struct rmap_obj *robj,
			uint64_t *start, int num_pages, struct rmap_log *log)
{
	struct mcsqnode_t qnode;
	robj->node.start = *start;
	robj->node.last = *start + PAGE_SIZE * num_pages - 1;
	*start = robj->node.last;
	robj->time = get_time(read_tsc());
	for (;;) {
		if (mcslock_trylock(&log->lock, &qnode)) {
			list_add_tail(&robj->list_node, &log->head);
			mcslock_unlock(&log->lock, &qnode);
			break;
		}
	}
}

static void init_rlog(struct rmap_log *log)
{
	log->lock.qnode = NULL;
	INIT_LIST_HEAD(&log->head);
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
	struct rmap_log *my_rlog = &rlog[i];

	init_rlog(my_rlog);

	struct rmap_obj *objs = malloc(sizeof(struct rmap_obj) * niter);
	if (!objs)
		edie("cannot allocate rmap objs\n");

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
		if (k%rperc == 0) {
			down_write(&rb_sem);
			build_tree(&tree, rlog);
			/* XXX: can be downgrade_write() */
			up_write(&rb_sem);
			down_read(&rb_sem);
			if (k%20)
				search_tree(start + PAGE_SIZE * (k % 10),
					    PAGE_SIZE, &tree);
			else
				search_tree(mid + PAGE_SIZE * (k % 20),
					    PAGE_SIZE, &tree);
			up_read(&rb_sem);
		} else {
			/* down_read(&rb_sem); */
			if (k % 2)
				insert_tree(&tree, &objs[k], &bmid,
					    k % 10, my_rlog);
			else
				insert_tree(&tree, &objs[k], &bstart,
					    k % 200, my_rlog);
			/* up_read(&rb_sem); */
		}
	}
	down_write(&rb_sem);
	build_tree(&tree, rlog);
	up_write(&rb_sem);

	sync_state->cpu[i].cycle = read_tsc() - s;
	double sec = (usec() - t) / 1000000000.0;
	sync_state->cpu[i].tput = (double) niter / sec;
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
