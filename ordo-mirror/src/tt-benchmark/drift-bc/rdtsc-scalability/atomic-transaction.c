/*
 * Create threads, mmap non-overlapping address range, reference it, and unmap it.
 */

#include <sys/mman.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>

#include "bench.h"

static struct {
    volatile int start;
    volatile int last;
    union {
        struct {
            volatile int ready;
            volatile double tput;
        };
        char pad[CACHE_BYTES];
    } cpu[MAX_CPU] __attribute__((aligned(CACHE_BYTES)));
} *sync_state;

static int ncores;
static int time_elapsed = 0;
static uint64_t counter __attribute__((aligned(CACHE_BYTES)));
static int btime = 10;

#define __READ_ONCE_SIZE \
    ({ \
     switch (size) {                         \
     case 1: *(uint8_t *)res = *(volatile uint8_t *)p; break;      \
     case 2: *(uint16_t *)res = *(volatile uint16_t *)p; break;        \
     case 4: *(uint32_t *)res = *(volatile uint32_t *)p; break;        \
     case 8: *(uint64_t *)res = *(volatile uint64_t *)p; break;        \
     default:                            \
     barrier();                      \
     __builtin_memcpy((void *)res, (const void *)p, size);   \
     barrier();                      \
     } \
     })


#define READ_ONCE(x) \
    ({ \
     union {typeof(x) __val; char __c[1];} __u; \
     __read_once(&(x), __u.__c, sizeof(x)); \
     __u.__val; \
     })

static void __read_once(const volatile void *p, void *res, int size)
{
    __READ_ONCE_SIZE;
}

static void alarm_handler(int signal)
{
    time_elapsed = 1;
}

static void *worker(void *x)
{
    int i = (uintptr_t)x;
    uint64_t count = 0;

    setaffinity(i);
    sync_state->cpu[i].ready = 1;
    if (ncores == 1)
        sync_state->last = 1;
    if (i) {
        if (i == ncores - 1)
            sync_state->last = 1;
        while (!sync_state->start)
            nop_pause();

    } else {
        while (!sync_state->last)
            nop_pause();
        signal(SIGALRM, &alarm_handler);
        alarm(btime);
        sync_state->start = 1;
    }

    do {
        __sync_fetch_and_add(&counter, 1);
        (void)(READ_ONCE(counter));
        ++count;
    } while (!time_elapsed);

    sync_state->cpu[i].tput = (double) count;
    return 0;
}

static void waitup(void)
{
    int i;
    double avg = 0.0;

    for (i = 0; i < ncores; i++) {
        while (!sync_state->cpu[i].tput)
            nop_pause();
        avg += sync_state->cpu[i].tput;
    }

    printf("%d cores completed total tput %lf, avg: %.3g\n",
           ncores, avg, avg/(btime * ncores));
}

int main(int ac, char **av)
{
    int threads;
    pthread_t th;
    int i;

    setaffinity(0);
    if (ac < 1)
        die("usage: %s num-cores time", av[0]);

    ncores = atoi(av[1]);
    threads = 1;
    if (ac == 3)
        btime = atoi(av[2]);

    sync_state = (void *) mmap(0, sizeof(*sync_state),
                               PROT_READ | PROT_WRITE,
                               MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (sync_state == MAP_FAILED)
        edie("mmap sync_state failed");
    memset(sync_state, 0, sizeof(*sync_state));

    for (i = 1; i < ncores; i++) {
        if (threads) {
            if (pthread_create(&th, NULL, worker, (void *)(intptr_t)i) < 0)
                edie("pthread_create");
        } else {
            pid_t p = fork();
            if (p < 0)
                edie("fork");
            if (!p) {
                worker((void *)(intptr_t)i);
                return 0;
            }
        }

        while (!sync_state->cpu[i].ready)
            nop_pause();
    }

    worker((void *)(intptr_t)0);
    waitup();
    return 0;
}
