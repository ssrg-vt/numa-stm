#ifndef _GNU_SOURCE
#define _GNU_SOURCE 1
#endif

#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include <stdint.h>
#include <unistd.h>
#include <setjmp.h>

#include <pthread.h>
#include <signal.h>
#include <pthread.h>

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

#include <errno.h>


#include <assert.h>
#include <stdlib.h>
#include <sched.h>
#include <unistd.h>
#include <numa.h>
#include "thread.h"
#include "types.h"
#include "random.h"

static THREAD_LOCAL_T    global_threadId;
static long              global_numThread       = 1;
static THREAD_BARRIER_T* global_barrierPtr      = NULL;
static long*             global_threadIds       = NULL;
static THREAD_ATTR_T     global_threadAttr;
static THREAD_T*         global_threads         = NULL;
static void            (*global_funcPtr)(void*) = NULL;
static void*             global_argPtr          = NULL;
static volatile bool_t   global_doShutdown      = FALSE;

long totalThreads;
thread_args_t* threadArgsArr;
__thread volatile unsigned long* commits;
__thread volatile unsigned long* aborts;

int numberThreads;
THREAD_MUTEX_T the_lock;

__thread double prevSample = 0.0;
__thread double newSample = 0.0;

/* =============================================================================
 * threadWait
 * -- Synchronizes all threads to start/stop parallel section
 * =============================================================================
 */
static void
threadWait (void* argPtr)
{
    thread_args_t* args = (thread_args_t*) argPtr;
    long threadId = args->threadId;
    commits = &(args->commits);
    aborts = &(args->aborts);

//    //assume symmetric numa zones
//    printf("numa zones count = %d\n", numa_num_configured_nodes());
//	//pin the thread to the numa zone
//    bitmask* mask = numa_allocate_cpumask();
//
//    //get # of cpu per node
//    numa_node_to_cpus(0, mask);
//    int cpu_per_node = 0;
//    while(numa_bitmask_isbitset(mask, cpu_per_node)) {
//    	cpu_per_node++;
//    }
//
//    printf("cpus per node = %d\n", cpu_per_node);
//
//    numa_bitmask_clearall(mask);
//    numa_bitmask_setbit(mask, /*numa_zone * cpu_per_node + (id % (cpu_per_node-1)) +1*/ threadId);
//    if (numa_sched_setaffinity(0, mask)) {
//    	perror("numa_sched_setaffinity");
//		exit(-1);
//    }
//    numa_free_cpumask(mask);
//
//    int curcpu = sched_getcpu();
//    printf("cpu set to %d\n", curcpu);
  /*  		cpu_set_t mask;
            int status;

            CPU_ZERO(&mask);
            CPU_SET(threadId, &mask);
            status = sched_setaffinity(0, sizeof(mask), &mask);
            if (status != 0)
            {
                perror("sched_setaffinity");
            }
*/
    THREAD_LOCAL_SET(global_threadId, (long)threadId);

    bindThread(threadId);

    while (1) {
        THREAD_BARRIER(global_barrierPtr, threadId); /* wait for start parallel */
        if (global_doShutdown) {
            break;
        }
        global_funcPtr(global_argPtr);
        THREAD_BARRIER(global_barrierPtr, threadId); /* wait for end parallel */
        if (threadId == 0) {
            break;
        }
    }
}

/* =============================================================================
 * thread_startup
 * -- Create pool of secondary threads
 * -- numThread is total number of threads (primary + secondaries)
 * =============================================================================
 */
void
thread_startup (long numThread)
{
    long i;

    totalThreads = numThread;

    global_numThread = numThread;
    global_doShutdown = FALSE;

    /* Set up barrier */
    assert(global_barrierPtr == NULL);
    global_barrierPtr = THREAD_BARRIER_ALLOC(numThread);
    assert(global_barrierPtr);
    THREAD_BARRIER_INIT(global_barrierPtr, numThread);

    /* Set up ids */
    THREAD_LOCAL_INIT(global_threadId);
    assert(global_threadIds == NULL);
    global_threadIds = (long*)malloc(numThread * sizeof(long));
    assert(global_threadIds);
    for (i = 0; i < numThread; i++) {
        global_threadIds[i] = i;
    }

    /* Set up thread list */
    assert(global_threads == NULL);
    global_threads = (THREAD_T*)malloc(numThread * sizeof(THREAD_T));
    assert(global_threads);

    threadArgsArr = (thread_args_t*) malloc(numThread * sizeof(thread_args_t));
    threadArgsArr[0].aborts = 0;
    threadArgsArr[0].commits = 0;
    threadArgsArr[0].threadId = global_threadIds[0];

    /* Set up pool */
    THREAD_ATTR_INIT(global_threadAttr);
    for (i = 1; i < numThread; i++) {
        threadArgsArr[i].aborts = 0;
        threadArgsArr[i].commits = 0;
        threadArgsArr[i].threadId = global_threadIds[i];
        THREAD_CREATE(global_threads[i],
                      global_threadAttr,
                      &threadWait,
                      &(threadArgsArr[i]));
    }

    /*
     * Wait for primary thread to call thread_start
     */
}

typedef struct stats_block {
    double commits;
    unsigned long aborts;
    unsigned long blockId;
    struct stats_block* next;
} stats_block_t;

static volatile int continueProfiling = 1;
static volatile stats_block_t* head;

void periodic_profiler(void* args) {

    head = (stats_block_t*) malloc(sizeof(stats_block_t));
    stats_block_t* latest = head;
    latest->commits = 0;
    latest->aborts = 0;
    latest->blockId = 0;

    while (continueProfiling) {
        usleep(100000);
        int i;
        unsigned long totalCommits = 0;
        unsigned long totalAborts = 0;
        for (i = 0; i < totalThreads; i++) {
            totalCommits += threadArgsArr[i].commits;
            totalAborts += threadArgsArr[i].aborts;
        }
        //unsigned short retries = threadArgsArr[0].retries;
        //unsigned short ucb = threadArgsArr[0].ucb;
        stats_block_t* new1 = (stats_block_t*) malloc(sizeof(stats_block_t));
        new1->commits = (double)totalCommits;
        //new->aborts = totalAborts;
        //new->retries = retries;
        //new->ucb = ucb;
        new1->blockId = latest->blockId + 1;
        new1->next = NULL;
        latest->next = new1;
        latest = new1;
    }

}

/* =============================================================================
 * thread_start
 * -- Make primary and secondary threads execute work
 * -- Should only be called by primary thread
 * -- funcPtr takes one arguments: argPtr
 * =============================================================================
 */
void
thread_start (void (*funcPtr)(void*), void* argPtr)
{
    global_funcPtr = funcPtr;
    global_argPtr = argPtr;

    /* Profiling disabled* */
    // THREAD_T profiler;
    // THREAD_CREATE(profiler, global_threadAttr, &periodic_profiler, NULL);

    threadWait((void*)&(threadArgsArr[0]));

    continueProfiling = 0;

    /* stats_block_t* previous = head;
    while (head->next != NULL) {
        head = head->next;
        double throughput = (head->commits - previous->commits) / (1000000.0);
        printf("Profiling: %lu\t%lf\n", head->blockId, throughput);
        previous = head;
    } */
}


/* =============================================================================
 * thread_shutdown
 * -- Primary thread kills pool of secondary threads
 * =============================================================================
 */
void
thread_shutdown ()
{
    /* Make secondary threads exit wait() */
    global_doShutdown = TRUE;
    THREAD_BARRIER(global_barrierPtr, 0);

    long numThread = global_numThread;

    long i;
    for (i = 1; i < numThread; i++) {
        THREAD_JOIN(global_threads[i]);
    }

    THREAD_BARRIER_FREE(global_barrierPtr);
    global_barrierPtr = NULL;

    free(global_threadIds);
    global_threadIds = NULL;

    free(global_threads);
    global_threads = NULL;

    global_numThread = 1;
}

#ifdef LOG_BARRIER

/* =============================================================================
 * thread_barrier_alloc
 * =============================================================================
 */
thread_barrier_t*
thread_barrier_alloc (long numThread)
{
    thread_barrier_t* barrierPtr;

    assert(numThread > 0);
    assert((numThread & (numThread - 1)) == 0); /* must be power of 2 */
    barrierPtr = (thread_barrier_t*)malloc(numThread * sizeof(thread_barrier_t));
    if (barrierPtr != NULL) {
        barrierPtr->numThread = numThread;
    }

    return barrierPtr;
}


/* =============================================================================
 * thread_barrier_free
 * =============================================================================
 */
void
thread_barrier_free (thread_barrier_t* barrierPtr)
{
    free(barrierPtr);
}


/* =============================================================================
 * thread_barrier_init
 * =============================================================================
 */
void
thread_barrier_init (thread_barrier_t* barrierPtr)
{
    long i;
    long numThread = barrierPtr->numThread;

    for (i = 0; i < numThread; i++) {
        barrierPtr[i].count = 0;
        THREAD_MUTEX_INIT(barrierPtr[i].countLock);
        THREAD_COND_INIT(barrierPtr[i].proceedCond);
        THREAD_COND_INIT(barrierPtr[i].proceedAllCond);
    }
}


/* =============================================================================
 * thread_barrier
 * -- Simple logarithmic barrier
 * =============================================================================
 */
void
thread_barrier (thread_barrier_t* barrierPtr, long threadId)
{
    long i = 2;
    long base = 0;
    long index;
    long numThread = barrierPtr->numThread;

    if (numThread < 2) {
        return;
    }

    do {
        index = base + threadId / i;
        if ((threadId % i) == 0) {
            THREAD_MUTEX_LOCK(barrierPtr[index].countLock);
            barrierPtr[index].count++;
            while (barrierPtr[index].count < 2) {
                THREAD_COND_WAIT(barrierPtr[index].proceedCond,
                                 barrierPtr[index].countLock);
            }
            THREAD_MUTEX_UNLOCK(barrierPtr[index].countLock);
        } else {
            THREAD_MUTEX_LOCK(barrierPtr[index].countLock);
            barrierPtr[index].count++;
            if (barrierPtr[index].count == 2) {
                THREAD_COND_SIGNAL(barrierPtr[index].proceedCond);
            }
            while (THREAD_COND_WAIT(barrierPtr[index].proceedAllCond,
                                    barrierPtr[index].countLock) != 0)
            {
                /* wait */
            }
            THREAD_MUTEX_UNLOCK(barrierPtr[index].countLock);
            break;
        }
        base = base + numThread / i;
        i *= 2;
    } while (i <= numThread);

    for (i /= 2; i > 1; i /= 2) {
        base = base - numThread / i;
        index = base + threadId / i;
        THREAD_MUTEX_LOCK(barrierPtr[index].countLock);
        barrierPtr[index].count = 0;
        THREAD_COND_SIGNAL(barrierPtr[index].proceedAllCond);
        THREAD_MUTEX_UNLOCK(barrierPtr[index].countLock);
    }
}

#else

barrier_t *barrier_alloc() {
    return (barrier_t *)malloc(sizeof(barrier_t));
}

void barrier_free(barrier_t *b) {
    free(b);
}

void barrier_init(barrier_t *b, int n) {
    pthread_cond_init(&b->complete, NULL);
    pthread_mutex_init(&b->mutex, NULL);
    b->count = n;
    b->crossing = 0;
}

void barrier_cross(barrier_t *b) {
    pthread_mutex_lock(&b->mutex);
    /* One more thread through */
    b->crossing++;
    /* If not all here, wait */
    if (b->crossing < b->count) {
        pthread_cond_wait(&b->complete, &b->mutex);
    } else {
        /* Reset for next time */
        b->crossing = 0;
        pthread_cond_broadcast(&b->complete);
    }
    pthread_mutex_unlock(&b->mutex);
}


#endif /* !LOG_BARRIER */

/* =============================================================================
 * thread_barrier_wait
 * -- Call after thread_start() to synchronize threads inside parallel region
 * =============================================================================
 */
void
thread_barrier_wait()
{
#ifndef SIMULATOR
    long threadId = thread_getId();
#endif /* !SIMULATOR */
    THREAD_BARRIER(global_barrierPtr, threadId);
}


/* =============================================================================
 * thread_getId
 * -- Call after thread_start() to get thread ID inside parallel region
 * =============================================================================
 */
long
thread_getId()
{
    return (long)THREAD_LOCAL_GET(global_threadId);
}


/* =============================================================================
 * thread_getNumThread
 * -- Call after thread_start() to get number of threads inside parallel region
 * =============================================================================
 */
long
thread_getNumThread()
{
    return global_numThread;
}



/* =============================================================================
 * TEST_THREAD
 * =============================================================================
 */
#ifdef TEST_THREAD


#include <stdio.h>
#include <unistd.h>


#define NUM_THREADS    (4)
#define NUM_ITERATIONS (3)



void
printId (void* argPtr)
{
    long threadId = thread_getId();
    long numThread = thread_getNumThread();
    long i;

    for ( i = 0; i < NUM_ITERATIONS; i++ ) {
        thread_barrier_wait();
        if (threadId == 0) {
            sleep(1);
        } else if (threadId == numThread-1) {
            usleep(100);
        }
        printf("i = %li, tid = %li\n", i, threadId);
        if (threadId == 0) {
            puts("");
        }
        fflush(stdout);
    }
}


int
main ()
{
    puts("Starting...");

    /* Run in parallel */
    thread_startup(NUM_THREADS);
    /* Start timing here */
    thread_start(printId, NULL);
    thread_start(printId, NULL);
    thread_start(printId, NULL);
    /* Stop timing here */
    thread_shutdown();

    puts("Done.");

    return 0;
}


#endif /* TEST_THREAD */


/* =============================================================================
 *
 * End of thread.c
 *
 * =============================================================================
 */
