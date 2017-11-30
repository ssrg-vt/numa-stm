#ifndef THREAD_H
#define THREAD_H 1


#include <pthread.h>
#include <stdlib.h>
#include "types.h"
#ifdef OTM
#include "omp.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif


#define THREAD_T                            pthread_t
#define THREAD_ATTR_T                       pthread_attr_t

#define THREAD_ATTR_INIT(attr)              pthread_attr_init(&attr)
#define THREAD_JOIN(tid)                    pthread_join(tid, (void**)NULL)
#define THREAD_CREATE(tid, attr, fn, arg)   pthread_create(&(tid), \
                                                           &(attr), \
                                                           (void* (*)(void*))(fn), \
                                                           (void*)(arg))

#define THREAD_LOCAL_T                      pthread_key_t
#define THREAD_LOCAL_INIT(key)              pthread_key_create(&key, NULL)
#define THREAD_LOCAL_SET(key, val)          pthread_setspecific(key, (void*)(val))
#define THREAD_LOCAL_GET(key)               pthread_getspecific(key)

#define THREAD_MUTEX_T                      pthread_mutex_t
#define THREAD_MUTEX_INIT(lock)             pthread_mutex_init(&(lock), NULL)
#define THREAD_MUTEX_LOCK(lock)             pthread_mutex_lock(&(lock))
#define THREAD_MUTEX_UNLOCK(lock)           pthread_mutex_unlock(&(lock))

#define THREAD_COND_T                       pthread_cond_t
#define THREAD_COND_INIT(cond)              pthread_cond_init(&(cond), NULL)
#define THREAD_COND_SIGNAL(cond)            pthread_cond_signal(&(cond))
#define THREAD_COND_BROADCAST(cond)         pthread_cond_broadcast(&(cond))
#define THREAD_COND_WAIT(cond, lock)        pthread_cond_wait(&(cond), &(lock))

#ifdef SIMULATOR
#  define THREAD_BARRIER_T                  pthread_barrier_t
#  define THREAD_BARRIER_ALLOC(N)           ((THREAD_BARRIER_T*)malloc(sizeof(THREAD_BARRIER_T)))
#  define THREAD_BARRIER_INIT(bar, N)       pthread_barrier_init(bar, 0, N)
#  define THREAD_BARRIER(bar, tid)          pthread_barrier_wait(bar)
#  define THREAD_BARRIER_FREE(bar)          free(bar)
#else /* !SIMULATOR */

#ifdef LOG_BARRIER
#  define THREAD_BARRIER_T                  thread_barrier_t
#  define THREAD_BARRIER_ALLOC(N)           thread_barrier_alloc(N)
#  define THREAD_BARRIER_INIT(bar, N)       thread_barrier_init(bar)
#  define THREAD_BARRIER(bar, tid)          thread_barrier(bar, tid)
#  define THREAD_BARRIER_FREE(bar)          thread_barrier_free(bar)
#else
#  define THREAD_BARRIER_T                  barrier_t
#  define THREAD_BARRIER_ALLOC(N)           barrier_alloc()
#  define THREAD_BARRIER_INIT(bar, N)       barrier_init(bar, N)
#  define THREAD_BARRIER(bar, tid)          barrier_cross(bar)
#  define THREAD_BARRIER_FREE(bar)          barrier_free(bar)
#endif /* !LOG_BARRIER */
#endif /* !SIMULATOR */

#include "random.h"

extern int numberThreads;

typedef struct thread_args {
    volatile unsigned long commits;
    volatile unsigned long aborts;
    long threadId;
} thread_args_t;

extern long totalThreads;
extern thread_args_t* threadArgsArr;

extern __thread volatile unsigned long* commits;
extern __thread volatile unsigned long* aborts;

extern THREAD_MUTEX_T the_lock;


#ifdef LOG_BARRIER
typedef struct thread_barrier {
    THREAD_MUTEX_T countLock;
    THREAD_COND_T proceedCond;
    THREAD_COND_T proceedAllCond;
    long count;
    long numThread;
} thread_barrier_t;
#else

typedef struct barrier {
    pthread_cond_t complete;
    pthread_mutex_t mutex;
    int count;
    int crossing;
} barrier_t;

barrier_t *barrier_alloc();

void barrier_free(barrier_t *b);

void barrier_init(barrier_t *b, int n);

void barrier_cross(barrier_t *b);

#endif /* LOG_BARRIER */


/* =============================================================================
 * thread_startup
 * -- Create pool of secondary threads
 * -- numThread is total number of threads (primary + secondary)
 * =============================================================================
 */
void
thread_startup (long numThread);


/* =============================================================================
 * thread_start
 * -- Make primary and secondary threads execute work
 * -- Should only be called by primary thread
 * -- funcPtr takes one arguments: argPtr
 * =============================================================================
 */
void
thread_start (void (*funcPtr)(void*), void* argPtr);


/* =============================================================================
 * thread_shutdown
 * -- Primary thread kills pool of secondary threads
 * =============================================================================
 */
void
thread_shutdown ();


#ifdef LOG_BARRIER
/* =============================================================================
 * thread_barrier_alloc
 * =============================================================================
 */
thread_barrier_t*
thread_barrier_alloc (long numThreads);


/* =============================================================================
 * thread_barrier_free
 * =============================================================================
 */
void
thread_barrier_free (thread_barrier_t* barrierPtr);


/* =============================================================================
 * thread_barrier_init
 * =============================================================================
 */
void
thread_barrier_init (thread_barrier_t* barrierPtr);


/* =============================================================================
 * thread_barrier
 * -- Simple logarithmic barrier
 * =============================================================================
 */
void
thread_barrier (thread_barrier_t* barrierPtr, long threadId);

#endif /* LOG_BARRIER */


/* =============================================================================
 * thread_barrier_wait
 * -- Call after thread_start() to synchronize threads inside parallel region
 * =============================================================================
 */
void
thread_barrier_wait();

/* =============================================================================
 * thread_getId
 * -- Call after thread_start() to get thread ID inside parallel region
 * =============================================================================
 */
long
thread_getId();


/* =============================================================================
 * thread_getNumThread
 * -- Call after thread_start() to get number of threads inside parallel region
 * =============================================================================
 */
long
thread_getNumThread();



#ifdef __cplusplus
}
#endif


#endif /* THREAD_H */


/* =============================================================================
 *
 * End of thread.h
 *
 * =============================================================================
 */
