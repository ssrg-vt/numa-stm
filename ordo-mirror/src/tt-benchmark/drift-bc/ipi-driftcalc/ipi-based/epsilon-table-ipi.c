#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kthread.h>
#include <linux/slab.h>
#include <linux/percpu.h>
#include <linux/spinlock.h>
#include "cpuseq.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("sslab <sslab@gatech.edu>");

#define ITERS 100000

struct timestamp {
    int index;
    u64 *tvalue;
};

static __always_inline unsigned long long rdtscp(void)
{
    DECLARE_ARGS(val, low, high);

    asm volatile("rdtscp" : EAX_EDX_RET(val, low, high));

    return EAX_EDX_VAL(val, low, high);
}

static __always_inline unsigned long long rdtscp_ordered(void)
{
    alternative_2("", "mfence", X86_FEATURE_MFENCE_RDTSC,
                  "lfence", X86_FEATURE_LFENCE_RDTSC);
    return rdtscp();
}

static DEFINE_PER_CPU(struct timestamp, timestamp);

static struct task_struct *thread;

static void init_timestamp_struct(void *data)
{
    struct timestamp *t = this_cpu_ptr(&timestamp);
    t->index = 0;
    t->tvalue = kzalloc_node(sizeof(u64) * ITERS, GFP_ATOMIC, numa_node_id());
}

static void collect_timestamp(void *data)
{
    struct timestamp *t = this_cpu_ptr(&timestamp);
    t->tvalue[t->index] = rdtscp_ordered();
    ++t->index;
}

void print_stats(int total)
{
    int i, cpu1, cpu2;
    u64 max_epsilon = 0;
    s64 min_epsilon = 1ULL << 31;
    s64 diff;

    for (i = 0; i < total; ++i) {
        for_each_online_cpu(cpu1) {
            if (cpu1 < online_cpus)
                break;
            for_each_online_cpu(cpu2) {
                struct timestamp *t1, *t2;

                if (cpu2 < online_cpus)
                    break;

                if (cpu1 >= cpu2)
                    break;

                t1 = per_cpu_ptr(&timestamp, cpu1);
                t2 = per_cpu_ptr(&timestamp, cpu2);

                if (t2->tvalue[i] > t1->tvalue[i])
                    diff = t2->tvalue[i] - t1->tvalue[i];
                else
                    diff = t1->tvalue[i] - t2->tvalue[i];

                if (diff < max_epsilon)
                    max_epsilon = diff;

                printk(KERN_INFO "iter: %d (%d, %d): %Ld\n",
                       i, cpu1, cpu2, diff);
            }
        }
        if (min_epsilon > max_epsilon)
            min_epsilon = max_epsilon;
    }
    printk(KERN_INFO "epsilon: %Ld\n", min_epsilon);
}

int thread_fn(void *data)
{
    int cpu, iters;

    for_each_online_cpu(cpu) {
        smp_call_function_single(cpuseq[cpu], init_timestamp_struct, NULL, 1);
    }

    for (iters = 0; iters < ITERS; ++iters)
        smp_call_function_many(cpu_online_mask, collect_timestamp, NULL, 1);

    do_exit(0);
    return 0;
}

static void epsilon_table_exit(void)
{
    return;
}

static int __init epsilon_table_init(void)
{
    int cpu = cpuseq[0];
    char t[8] = "thread";
    thread = kthread_create(thread_fn, NULL, t);
    if (thread) {
        kthread_bind(thread, cpu);
        wake_up_process(thread);
    }
    return 0;
}

module_init(epsilon_table_init);
module_exit(epsilon_table_exit);
