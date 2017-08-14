#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <string.h>

#include "config.h"

#if defined (__SVR4) && defined (__sun)
#include <sys/mman.h>
#include <sys/inttypes.h>
#include <unistd.h>
#else
#include <sys/user.h>
#include <inttypes.h>
#endif

#define __noret__ __attribute__((noreturn))
#define __align__ __attribute__((aligned(CACHE_BYTES)))

#define smp_cas(__ptr, __old_val, __new_val)	\
	__sync_bool_compare_and_swap(__ptr, __old_val, __new_val)

#define smp_cas_val(__ptr, __old_val, __new_val)	\
	__sync_val_compare_and_swap(__ptr, __old_val, __new_val)

#define smp_swap(__ptr, __val)			\
	__sync_lock_test_and_set(__ptr, __val)


void __noret__ die(const char* errstr, ...);
void __noret__ edie(const char* errstr, ...);

void setaffinity(int c);

uint64_t usec(void);

static inline uint64_t read_tsc(void)
{
	uint32_t a, d;
	__asm __volatile("rdtsc" : "=a" (a), "=d" (d));
	return ((uint64_t) a) | (((uint64_t) d) << 32);
}

static inline uint64_t read_tscp(void)
{
	uint32_t a, d;
	__asm __volatile("rdtscp": "=a"(a), "=d"(d));
	return ((uint64_t) a) | (((uint64_t) d) << 32);
}

static inline void smp_wmb(void)
{
    __asm__ __volatile__("sfence":::"memory");
}

static inline void smp_rmb(void)
{
    __asm__ __volatile__("lfence":::"memory");
}

static inline unsigned int get_page_size(void)
{
#if defined (__SVR4) && defined (__sun)
	return sysconf(_SC_PAGE_SIZE);
#else
	return PAGE_SIZE;
#endif
}

static inline void * xmalloc(unsigned int sz)
{
	size_t s;
	void *r;
	
	s = ((sz - 1) + CACHE_BYTES) & ~(CACHE_BYTES - 1);
#if defined (__SVR4) && defined (__sun)
	r = memalign(s, CACHE_BYTES);
#else
	if (posix_memalign(&r, CACHE_BYTES, s))
		edie("posix_memalign");
#endif
	memset(r, 0, sz);
	return r;
}

static inline uint32_t rnd(uint32_t *seed)
{
	*seed = *seed * 1103515245 + 12345;
	return *seed & 0x7fffffff;
}

static inline void nop_pause(void)
{
	__asm __volatile("pause");
}

static inline void rep_nop(void)
{
	__asm __volatile("rep; nop" ::: "memory");
}

static inline void barrier(void)
{
	__asm __volatile("":::"memory");
}

static inline void cpu_relax(void)
{
	rep_nop();
}

#if ENABLE_PMC
static inline uint64_t read_pmc(uint32_t ecx)
{
	uint32_t a, d;
	__asm __volatile("rdpmc" : "=a" (a), "=d" (d) : "c" (ecx));
	return ((uint64_t) a) | (((uint64_t) d) << 32);
}
#else /* !ENABLE_PMC */
static inline uint64_t read_pmc(uint32_t ecx)
{
	return 0;
}
#endif

/* MCS queue lock */
struct mcsqnode_t;

struct mcslock_t {
	volatile struct mcsqnode_t *qnode;
};

struct mcsqnode_t {
	volatile int locked;
	struct mcsqnode_t *next;
};

static inline void mcslock_init(struct mcslock_t *lock)
{
	lock->qnode = NULL;
	smp_wmb();
}

static inline void mcslock_lock(struct mcslock_t *lock,
				struct mcsqnode_t *qnode)
{
	struct mcsqnode_t *prev;

	qnode->locked = 1;
	qnode->next = NULL;
	smp_wmb();

	prev = (struct mcsqnode_t *)smp_swap(&lock->qnode, qnode);
	if (prev) {
		prev->next = qnode;
		smp_wmb();
		while(qnode->locked)
			rep_nop();
	}
}

static inline int mcslock_trylock(struct mcslock_t *lock,
				  struct mcsqnode_t *qnode)
{
	qnode->next = NULL;
	if (smp_cas_val(&lock->qnode, NULL, qnode) == NULL) {
		qnode->locked = 0;
		return 1;
	}
	return 0;
}

static inline void mcslock_unlock(struct mcslock_t *lock,
				  struct mcsqnode_t *qnode)
{
	if (!qnode->next) {
		if (smp_cas(&lock->qnode, qnode, NULL))
			return;
		while (!qnode->next)
			smp_rmb();
	}
	qnode->next->locked = 0;
	smp_wmb();
}

static inline int mcslock_unlocked(struct mcslock_t *lock)
{
	return (lock->qnode == NULL);
}
