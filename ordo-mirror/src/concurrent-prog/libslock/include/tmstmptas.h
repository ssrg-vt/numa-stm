/*
 * File: tmstmptas.h
 * Author: Sanidhya Kashyap <sanidhya@gatech.edu>
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
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */


#ifndef _TMSTMPTAS_H_
#define _TMSTMPTAS_H_

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#  include <numa.h>
#include <pthread.h>
#include "utils.h"
#include "atomic_ops.h"
#include "padding.h"
#include <stdint.h>
#include "klist.h"

#define ____cacheline_aligned  __attribute__ ((aligned (CACHE_LINE_SIZE)))

#define NUMA_BATCH_SIZE             (128) /* per numa throughput */
#define NUMA_WAITING_SPINNERS       (4) /* spinners in the waiter spin phase */

#define UNLOCKED 	0
#define LOCKED 		1

struct nid_clock_info {
	uint64_t timestamp;
	int cid;
	int nid;
};

struct core_info {
	volatile uint64_t timestamp;
	volatile int32_t locked;
};

/* lock status */
typedef struct tmstmptaslock_t {
	volatile uint8_t lock;
	char pad0[63];
	struct core_info cinfo[NUMBER_OF_SOCKETS * CORES_PER_SOCKET];
	char pad1[32];
	volatile int32_t ssid;
	int batch_count;
} tmstmptaslock_t ____cacheline_aligned;

int tmstmptas_trylock(tmstmptaslock_t* lock);
void tmstmptaslock_acquire(tmstmptaslock_t* lock);
void tmstmptaslock_release(tmstmptaslock_t* lock);
int is_free_tmstmptas(tmstmptaslock_t* t);

int create_tmstmptaslock(tmstmptaslock_t* the_lock);
tmstmptaslock_t* init_tmstmptaslocks(uint32_t num_locks);
void init_thread_tmstmptaslocks(uint32_t thread_num);
void free_tmstmptaslocks(tmstmptaslock_t* the_locks);

#endif
