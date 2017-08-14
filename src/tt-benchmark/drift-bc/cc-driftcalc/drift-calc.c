#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kthread.h>
#include <linux/slab.h>
#include <linux/percpu.h>
#include <linux/spinlock.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("sslab <sslab@gatech.edu>");

#define CACHE_BYTES 64

#define PHASE_INIT 		1
#define PHASE_SEND_CORE1 	2
#define PHASE_RECV_CORE1	3
#define PHASE_RESET 		4
#define PHASE_END 		5

#if 1
struct rdtsc_pair {
	uint64_t t_1;
	uint64_t t_2;
} ____cacheline_aligned;
#endif

struct drift_pair {
	int64_t a;
	int64_t b;
	int64_t ldiff;
	int64_t rdiff;
};

static int niter = 100;
static int cpu0 = 0;
static int cpu1 = 1;
module_param(niter, int, S_IRUGO | S_IWUSR);
module_param(cpu0, int, S_IRUGO | S_IWUSR);
module_param(cpu1, int, S_IRUGO | S_IWUSR);

DEFINE_PER_CPU(struct rdtsc_pair, tsc_value);

static struct task_struct *threads[2];
static volatile int phase;
static struct drift_pair *diffs;

/* static __always_inline uint64_t rdtsc_ordered(void) */
/* { */
/* 	DECLARE_ARGS(val, low, high); */
/* 	asm volatile("rdtsc" : EAX_EDX_RET(val, low, high)); */
/* 	return EAX_EDX_VAL(val, low, high); */
/* } */

static void print_stats(void)
{
	int index;
	int64_t rtt = 0, offset = 0;
	int64_t avg_rtt = 0, avg_offset = 0;
	int zero_count = 0;

	for (index = 0; index < niter; ++index) {
		/* if (diffs[index].rdiff != 69) */
			/* continue; */
		count++;
		rtt = diffs[index].a - diffs[index].b;
		offset = (diffs[index].a + diffs[index].b);
		avg_rtt += rtt;
		if (rtt < 0)
			rtt *= -1;
		if (offset < 0)
			offset *= -1;
		avg_offset += offset;
		if (offset == 0) {
			printk(KERN_CRIT "local diff: %Ld, remote diff: %Ld\n",
			       diffs[index].ldiff, diffs[index].rdiff);
			++zero_count;
		} else {
			if (flag == 0) {
				printk(KERN_CRIT "-- local diff: %Ld, remote diff: %Ld --\n",
				       diffs[index].ldiff, diffs[index].rdiff);
				flag = 0;
			}
		}
	}
	printk(KERN_CRIT "OFST_CLC (%d -> %d): iters: %d rtt: %Ld off: %Ld "
	       "#num zero offset: %d\n", cpu0, cpu1, niter,
	       avg_rtt/niter, avg_offset/(2*niter), zero_count);
	printk(KERN_CRIT "only with 69 OFST_CLC (%d -> %d): iters: %d rtt: %Ld off: %Ld "
	       "#num zero offset: %d\n", cpu0, cpu1, count,
	       avg_rtt/count, avg_offset/(2*count), zero_count);
}

int remote_fn(void *data)
{
	struct rdtsc_pair *p;
	raw_spinlock_t lock;
	unsigned long flags;
	raw_spin_lock_init(&lock);

	raw_spin_lock_irqsave(&lock, flags);
	p = this_cpu_ptr(&tsc_value);
	for (;;) {
		phase = PHASE_INIT;

		while(phase != PHASE_SEND_CORE1);

		p->t_1 = rdtsc_ordered();
		/* asm volatile("pause":::"memory"); */
		p->t_2 = rdtsc_ordered();

		phase = PHASE_RECV_CORE1;

		raw_spin_unlock_irqrestore(&lock, flags);

		while(phase == PHASE_RECV_CORE1)
			cpu_relax_lowlatency();

		if (phase != PHASE_RESET)
			break;

		raw_spin_lock_irqsave(&lock, flags);
	}
	do_exit(0);
        return 0;
}

static void populate_rdtsc_value(void *data)
{
	struct rdtsc_pair *rp = this_cpu_ptr(&tsc_value);
	struct rdtsc_pair *d = (struct rdtsc_pair *)data;

	d->t_1 = rp->t_1;
	d->t_2 = rp->t_2;
}

static void collect_stats(int index)
{
	struct rdtsc_pair rp, *lp;

	lp = this_cpu_ptr(&tsc_value);
	smp_call_function_single(cpu1, populate_rdtsc_value, &rp, 1);

	diffs[index].a = rp.t_1 - lp->t_1;
	diffs[index].b = rp.t_2 - lp->t_2;
	diffs[index].ldiff = lp->t_2 - lp->t_1;
	diffs[index].rdiff = rp.t_2 - rp.t_1;
}

int local_fn(void *data)
{
	int iter = 0;
	struct rdtsc_pair *p;
	raw_spinlock_t lock;
	unsigned long flags;


	raw_spin_lock_init(&lock);

	raw_spin_lock_irqsave(&lock, flags);
	p = this_cpu_ptr(&tsc_value);
	for (; iter < niter; ++iter) {

		while(phase != PHASE_INIT || phase == PHASE_RESET);

		p->t_1 = rdtsc_ordered();

		phase = PHASE_SEND_CORE1;

		while(phase != PHASE_RECV_CORE1);

		p->t_2 = rdtsc_ordered();

		raw_spin_unlock_irqrestore(&lock, flags);
		collect_stats(iter);
		raw_spin_lock_irqsave(&lock, flags);

		if (iter != niter - 1)
			phase = PHASE_RESET;
	}
	raw_spin_unlock_irqrestore(&lock, flags);
	phase = PHASE_END;

	print_stats();
	do_exit(0);
	return 0;
}

static void cc_drift_cal_exit(void)
{
        return;
}

static int __init cc_drift_cal_init(void)
{
	char t[8] = "thread";

	if (cpu0 == cpu1)
		return 0;

	if (niter <= 0)
		niter = 100;

	diffs = kmalloc(niter * sizeof(struct drift_pair), GFP_KERNEL);
	if (diffs == NULL)
		return ENOMEM;

	memset(diffs, 0, sizeof(struct drift_pair) * niter);

        threads[0] = kthread_create(local_fn, NULL, t);
        if (!threads[0])
		goto err;

	threads[1] = kthread_create(remote_fn, NULL, t);
        if (!threads[1])
		goto err;

	kthread_bind(threads[0], cpu0);
	kthread_bind(threads[1], cpu1);
	wake_up_process(threads[0]);
	wake_up_process(threads[1]);

        return 0;
     err:
	kfree(diffs);
	return EINVAL;
}

module_init(cc_drift_cal_init);
module_exit(cc_drift_cal_exit);
