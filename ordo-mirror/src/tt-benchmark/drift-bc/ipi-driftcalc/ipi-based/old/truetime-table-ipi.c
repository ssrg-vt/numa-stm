#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kthread.h>
#include <linux/slab.h>
#include <linux/percpu.h>
#include <linux/spinlock.h>
#include "cpuseq.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("sslab <sslab@gatech.edu>");

#define ITERS 100

static struct task_struct *thread;

struct rdtsc_pair {
	unsigned long long t_1;
	unsigned long long t_2;
};

DEFINE_PER_CPU(struct rdtsc_pair, rcpu_stats);

struct rdtsc_stats {
	struct rdtsc_pair ccstats;
	struct rdtsc_pair rcstats;
};

static void get_rdtsc_value(void *data)
{
	struct rdtsc_pair *rp = this_cpu_ptr(&rcpu_stats);

	rp->t_1 = rdtsc_ordered();
	rp->t_2 = rdtsc_ordered();
}

static void populate_rdtsc_value(void *data)
{
	struct rdtsc_pair *rp = this_cpu_ptr(&rcpu_stats);
	struct rdtsc_pair *sarray = (struct rdtsc_pair *)data;
	sarray->t_1 = rp->t_1;
	sarray->t_2 = rp->t_2;
}

static void populate_all_values(unsigned long long t_1, unsigned long long t_2,
				struct rdtsc_stats *rstats, int cpu)
{

	smp_call_function_single(cpuseq[cpu], populate_rdtsc_value,
				 &rstats->rcstats, 1);
	rstats->ccstats.t_1 = t_1;
	rstats->ccstats.t_2 = t_2;
}

void print_stats(struct rdtsc_stats *rstats, int cpu_id, int index)
{
	int iter = 0;
	unsigned long long rtt = 0, offset = 0;
	unsigned long long avg_rtt = 0, avg_offset = 0;

	for (iter = 1; iter < ITERS; ++iter) {
		/*
		 * rtt = (t4 - t1) - (t3 - t2)
		 */
		rtt = (rstats[index].ccstats.t_2 - rstats[index].ccstats.t_1) -
			(rstats[index].rcstats.t_2 - rstats[index].rcstats.t_1);
		offset = rstats[index].rcstats.t_1 - rstats[index].ccstats.t_1 -
			(rtt / 2);
		avg_rtt += rtt;
		avg_offset += offset;
		++index;
	}
	avg_rtt /= ITERS;
	avg_offset /= ITERS;
	printk(KERN_INFO "CPU: %d, AVG RTT: %Lu AVG OFFSET: %Lu (ns)\n", cpu_id,
	       avg_rtt, avg_offset);
}

int thread_fn(void *data)
{
        int cpu, iters;
	size_t size;
	struct rdtsc_stats *rstats;
	unsigned long long t_1, t_2;
	unsigned long long index = 0;
	spinlock_t lock;
	unsigned long flags;

	size = sizeof(struct rdtsc_stats) * ITERS * online_cpus;

	rstats = kmalloc(size, GFP_KERNEL);
	if (!rstats) {
		printk(KERN_INFO "memory allocation failed\n");
		goto out;
	}
	memset(rstats, 0, size);
	spin_lock_init(&lock);

        for (cpu = 1; cpu < online_cpus; cpu++) {
		spin_lock_irqsave(&lock, flags);
                for (iters = 0; iters < ITERS; ++iters) {
			t_1 = rdtsc_ordered();
			smp_call_function_single(cpuseq[cpu], get_rdtsc_value,
						 &t_1, 1);
			t_2 = rdtsc_ordered();
			populate_all_values(t_1, t_2, &rstats[index], cpu);
			index++;
		}
		spin_unlock_irqrestore(&lock, flags);
		print_stats(rstats, cpu, index - ITERS + 1);
        }
	kfree(rstats);
     out:
	printk(KERN_INFO "done\n");
        do_exit(0);
        return 0;
}

static void ipi_cost_cal_exit(void)
{
        return;
}

static int __init ipi_cost_cal_init(void)
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

module_init(ipi_cost_cal_init);
module_exit(ipi_cost_cal_exit);
