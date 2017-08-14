/*
 * File: tmstmptas.c
 * Author: Sanidhya Kashyap <sanidya@gatech.edu>
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2016 Sanidhya Kashyap
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * lockIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <stdarg.h>
#include <sys/types.h>

#include "tmstmptas.h"
#include "platform_defs.h"

#define smp_swap(__ptr, __val)                                                 \
	__sync_lock_test_and_set(__ptr, __val)
#define smp_cas(__ptr, __oval, __nval)                                         \
	__sync_bool_compare_and_swap(__ptr, __oval, __nval)
#define smp_faa(__ptr, __val)                                                  \
	__sync_fetch_and_add(__ptr, __val)

#define min(a, b) ((a)<(b)?(a):(b))
#define ACCESS_ONCE(x) (*(__volatile__ __typeof__(x) *)&(x))

static inline void smp_mb(void)
{
	__asm__ __volatile__("mfence":::"memory");
}

static inline void smp_rmb(void)
{
	__asm__ __volatile__("lfence":::"memory");
}

static inline void smp_wmb(void)
{
	__asm__ __volatile__("sfence":::"memory");
}

static inline void barrier(void)
{
	__asm__ __volatile__("":::"memory");
}

static uint64_t __always_inline rdtscp(void)
{
	uint32_t a, d, c;
	__asm__ volatile("rdtscp" : "=a"(a), "=d"(d), "=c"(c));
	return ((uint64_t) a) | (((uint64_t) d) << 32);
}

static void __always_inline numa_get_nid(struct nid_clock_info *v)
{
    uint32_t a, d, c;
    __asm__ volatile("rdtscp" : "=a"(a), "=d"(d), "=c"(c));

    /* nid must be positive. */
    v->cid = c & 0xFFF;
    v->nid = ((c & 0xFFF) / CORES_PER_SOCKET);
    v->timestamp = (uint64_t)a | (((uint64_t)d) << 32);
}


int tmstmptas_trylock(tmstmptaslock_t *lock)
{
	if (TAS_U8(&(lock->lock)) == 0)
		return 0;
	return 1;
}

void tmstmptaslock_acquire(tmstmptaslock_t* lock)
{
	struct nid_clock_info v;
	struct core_info *c;

	numa_get_nid(&v);
	c = &lock->cinfo[v.cid];
	c->locked = 0;

	if (TAS_U8(&lock->lock) == UNLOCKED)
		return;

	c->timestamp = v.timestamp;
	smp_wmb();

	for (;;) {
		int count = 0;
		if (lock->lock == UNLOCKED &&
			TAS_U8(&lock->lock) == UNLOCKED)
				break;
		for (count = 0; count < 1000; ++count) {
			if (c->locked)
				goto out;
		}
		smp_rmb();
	}
	lock->batch_count = 0;
	lock->ssid = v.nid;
     out:
	c->timestamp = 0;
}

void tmstmptaslock_release(tmstmptaslock_t* lock)
{
	int start_cid, i, min_cid = -1;
	uint64_t min_time = 1ULL << 62;

	start_cid = lock->ssid * CORES_PER_SOCKET;

	COMPILER_BARRIER;
	if (lock->batch_count >= NUMA_BATCH_SIZE) {
		goto out;
	}
	for (i = start_cid; i < start_cid + CORES_PER_SOCKET; ++i) {
		if (lock->cinfo[i].timestamp == 0)
			continue;
		if (min_time > lock->cinfo[i].timestamp) {
			min_time = lock->cinfo[i].timestamp;
			min_cid = i;
		}
	}
	if (min_cid != -1) {
		lock->cinfo[min_cid].locked = 1;
		++lock->batch_count;
		return;
	}
     out:
	lock->lock = UNLOCKED;
}

int create_tmstmptaslock(tmstmptaslock_t* the_lock)
{
	the_lock->lock = 0;
	memset(the_lock->cinfo, 0, sizeof(the_lock->cinfo));
	the_lock->ssid = -1;
	the_lock->batch_count = 0;
	smp_wmb();
	return 0;
}


void init_thread_tmstmptaslocks(uint32_t thread_num)
{
	set_cpu(thread_num);
}

tmstmptaslock_t *init_tmstmptaslocks(uint32_t num_locks)
{
	tmstmptaslock_t *the_locks;
	uint32_t i;
	the_locks = (tmstmptaslock_t *)malloc(sizeof(tmstmptaslock_t) *
					      num_locks);

	for (i = 0; i < num_locks; ++i) {
		the_locks[i].lock = 0;
		memset(the_locks[i].cinfo, 0, sizeof(the_locks[i].cinfo));
		the_locks[i].ssid = -1;
		the_locks[i].batch_count = 0;
	}
	smp_wmb();
	return the_locks;
}

void free_tmstmptaslocks(tmstmptaslock_t* the_locks)
{
	free(the_locks);
}
