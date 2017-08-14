#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kthread.h>
#include <linux/slab.h>
#include <linux/percpu.h>
#include <linux/spinlock.h>
#include <linux/smp.h>
#include <linux/sched.h>
#include <linux/irq_work.h>

#include "cpuseq.h"
#include "common.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("sslab <sslab@gatech.edu>");

static int wait = 0;

module_param(wait, int, S_IRUGO | S_IWUSR);

enum {
	CSD_FLAG_LOCK		= 0x01,
	CSD_FLAG_SYNCHRONOUS	= 0x02,
};

struct call_function_data {
	struct call_single_data	__percpu *csd;
	cpumask_var_t		cpumask;
};

static struct task_struct *thread;

/*
 * global ipi specific data from smp.c
 */
static DEFINE_PER_CPU_SHARED_ALIGNED(struct call_function_data, cfd_data);

/*
 * time keeping stuff
 */
static DEFINE_PER_CPU_SHARED_ALIGNED(struct rdtsc_pair, percpu_rdtsc_stats);

static inline unsigned long long rdtscp(void)
{
    unsigned long long a, d;
    __asm__ volatile("rdtscp" : "=a"(a), "=d"(d));
    return a | (d << 32UL);
}

static int local_smpcfd_prepare_cpu(unsigned int cpu)
{
	struct call_function_data *cfd = &per_cpu(cfd_data, cpu);

	if (!zalloc_cpumask_var_node(&cfd->cpumask, GFP_KERNEL,
				     cpu_to_node(cpu)))
		return -ENOMEM;
	cfd->csd = alloc_percpu(struct call_single_data);
	if (!cfd->csd) {
		free_cpumask_var(cfd->cpumask);
		return -ENOMEM;
	}

	return 0;
}

static int init_basic_ipi_infra(void)
{
	int i;
	int ret = 0;

	for_each_possible_cpu(i) {
		ret = local_smpcfd_prepare_cpu(i);
		if (ret)
			goto out;
	}
     out:
	return ret;
}

static __always_inline void csd_lock_wait(struct call_single_data *csd)
{
	smp_cond_load_acquire(&csd->flags, !(VAL & CSD_FLAG_LOCK));
}

static __always_inline void csd_lock(struct call_single_data *csd)
{
	csd_lock_wait(csd);
	csd->flags |= CSD_FLAG_LOCK;

	/*
	 * prevent CPU from reordering the above assignment
	 * to ->flags with any subsequent assignments to other
	 * fields of the specified call_single_data structure:
	 */
	smp_wmb();
}

static __always_inline void csd_unlock(struct call_single_data *csd)
{
	WARN_ON(!(csd->flags & CSD_FLAG_LOCK));

	/*
	 * ensure we're all done before releasing data:
	 */
	smp_store_release(&csd->flags, 0);
}

static void smp_call_function_all_fast(const struct cpumask *mask,
				       smp_call_func_t func, void *info,
				       bool wait, struct rdtsc_pair *rp)
{
	struct call_function_data *cfd = this_cpu_ptr(&cfd_data);

	/* Send a message to all CPUs in the map */
	rp->t_1 = rdtscp();
	arch_send_call_function_ipi_mask(cfd->cpumask);

	if (wait) {
		for_each_cpu(cpu, cfd->cpumask) {
			struct call_single_data *csd;

			csd = per_cpu_ptr(cfd->csd, cpu);
			csd_lock_wait(csd);
		}
	}
	rp->t_2 = rdtscp();
}

static void setup_smpcfd_struct(const struct cpumask *mask,
				smp_call_func_t func, void *info, bool wait)
{
	struct call_function_data *cfd;
	int cpu, next_cpu, this_cpu = smp_processor_id();

	cfd = this_cpu_ptr(&cfd_data);

	cpumask_and(cfd->cpumask, mask, cpu_online_mask);
	cpumask_clear_cpu(this_cpu, cfd->cpumask);

	for_each_cpu(cpu, cfd->cpumask) {
		struct call_single_data *csd = per_cpu_ptr(cfd->csd, cpu);

		if (wait)
			csd->flags |= CSD_FLAG_SYNCHRONOUS;
		csd->func = func;
		csd->info = info;
	}
}

int calc_drift_func(void *data)
{
	int ret = 0;

	ret = init_basic_ipi_infra();
	if (ret)
		goto out;

	setup_smpcfd_struct(cpu_online_mask, update_rdtsc_value, NULL, wait);

     out:
	return ret;
}

static void ipi_broadcast_time_exit(void)
{
        return;
}

static int __init ipi_broadcast_time_init(void)
{
	/* everything starts from 0, this will lead the way */
        int cpu = 0;

	char t[10] = "driftcalc";
        thread = kthread_create(calc_drift_func, NULL, t);
        if (thread) {
                kthread_bind(thread, cpu);
                wake_up_process(thread);
        }
        return 0;
}

module_init(ipi_broadcast_time_init);
module_exit(ipi_broadcast_time_exit);
