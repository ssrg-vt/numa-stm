/*************************************************************/
// Seqlock implementation
/*************************************************************/

#include "arch.h"

#define spinlock_t pthread_spinlock_t
#define spin_lock(l) pthread_spin_lock(l)
#define spin_unlock(l) pthread_spin_unlock(l)

typedef struct seqcount {
	unsigned sequence;
} seqcount_t;

static inline void __seqcount_init(seqcount_t *s)
{
	s->sequence = 0;
}

#define seqcount_init(s) __seqcount_init(s)

/**
 * __read_seqcount_begin - begin a seq-read critical section (without barrier)
 * @s: pointer to seqcount_t
 * Returns: count to be passed to read_seqcount_retry
 *
 * __read_seqcount_begin is like read_seqcount_begin, but has no smp_rmb()
 * barrier. Callers should ensure that smp_rmb() or equivalent ordering is
 * provided before actually loading any of the variables that are to be
 * protected in this critical section.
 *
 * Use carefully, only in critical code, and comment how the barrier is
 * provided.
 */
inline unsigned __read_seqcount_begin(const seqcount_t *s)
{
	unsigned ret;

repeat:
	ret = (*(volatile unsigned *)&(s->sequence));
	if ((ret & 1)) {
		cpu_relax();
		goto repeat;
	}
	return ret;
}

/**
 * raw_read_seqcount - Read the raw seqcount
 * @s: pointer to seqcount_t
 * Returns: count to be passed to read_seqcount_retry
 *
 * raw_read_seqcount opens a read critical section of the given
 * seqcount without any lockdep checking and without checking or
 * masking the LSB. Calling code is responsible for handling that.
 */
inline unsigned raw_read_seqcount(const seqcount_t *s)
{
    unsigned ret = (*(volatile unsigned *)&(s->sequence));
	smp_rmb();
	return ret;
}

/**
 * raw_read_seqcount_begin - start seq-read critical section w/o lockdep
 * @s: pointer to seqcount_t
 * Returns: count to be passed to read_seqcount_retry
 *
 * raw_read_seqcount_begin opens a read critical section of the given
 * seqcount, but without any lockdep checking. Validity of the critical
 * section is tested by checking read_seqcount_retry function.
 */
inline unsigned raw_read_seqcount_begin(const seqcount_t *s)
{
	unsigned ret = __read_seqcount_begin(s);
	smp_rmb();
	return ret;
}

/**
 * read_seqcount_begin - begin a seq-read critical section
 * @s: pointer to seqcount_t
 * Returns: count to be passed to read_seqcount_retry
 *
 * read_seqcount_begin opens a read critical section of the given seqcount.
 * Validity of the critical section is tested by checking read_seqcount_retry
 * function.
 */
inline unsigned read_seqcount_begin(const seqcount_t *s)
{
	return raw_read_seqcount_begin(s);
}

/**
 * raw_seqcount_begin - begin a seq-read critical section
 * @s: pointer to seqcount_t
 * Returns: count to be passed to read_seqcount_retry
 *
 * raw_seqcount_begin opens a read critical section of the given seqcount.
 * Validity of the critical section is tested by checking read_seqcount_retry
 * function.
 *
 * Unlike read_seqcount_begin(), this function will not wait for the count
 * to stabilize. If a writer is active when we begin, we will fail the
 * read_seqcount_retry() instead of stabilizing at the beginning of the
 * critical section.
 */
inline unsigned raw_seqcount_begin(const seqcount_t *s)
{
    unsigned ret = (*(volatile unsigned *)&(s->sequence));
	smp_rmb();
	return ret & ~1;
}

/**
 * __read_seqcount_retry - end a seq-read critical section (without barrier)
 * @s: pointer to seqcount_t
 * @start: count, from read_seqcount_begin
 * Returns: 1 if retry is required, else 0
 *
 * __read_seqcount_retry is like read_seqcount_retry, but has no smp_rmb()
 * barrier. Callers should ensure that smp_rmb() or equivalent ordering is
 * provided before actually loading any of the variables that are to be
 * protected in this critical section.
 *
 * Use carefully, only in critical code, and comment how the barrier is
 * provided.
 */
inline int __read_seqcount_retry(const seqcount_t *s, unsigned start)
{
	return (s->sequence != start);
}

/**
 * read_seqcount_retry - end a seq-read critical section
 * @s: pointer to seqcount_t
 * @start: count, from read_seqcount_begin
 * Returns: 1 if retry is required, else 0
 *
 * read_seqcount_retry closes a read critical section of the given seqcount.
 * If the critical section was invalid, it must be ignored (and typically
 * retried).
 */
inline int read_seqcount_retry(const seqcount_t *s, unsigned start)
{
	smp_rmb();
	return __read_seqcount_retry(s, start);
}

inline void raw_write_seqcount_begin(seqcount_t *s)
{
	s->sequence++;
	smp_wmb();
}

inline void raw_write_seqcount_end(seqcount_t *s)
{
	smp_wmb();
	s->sequence++;
}

/**
 * raw_write_seqcount_barrier - do a seq write barrier
 * @s: pointer to seqcount_t
 *
 * This can be used to provide an ordering guarantee instead of the
 * usual consistency guarantee. It is one wmb cheaper, because we can
 * collapse the two back-to-back wmb()s.
 *
 *      seqcount_t seq;
 *      bool X = true, Y = false;
 *
 *      void read(void)
 *      {
 *              bool x, y;
 *
 *              do {
 *                      int s = read_seqcount_begin(&seq);
 *
 *                      x = X; y = Y;
 *
 *              } while (read_seqcount_retry(&seq, s));
 *
 *              BUG_ON(!x && !y);
 *      }
 *
 *      void write(void)
 *      {
 *              Y = true;
 *
 *              raw_write_seqcount_barrier(seq);
 *
 *              X = false;
 *      }
 */
inline void raw_write_seqcount_barrier(seqcount_t *s)
{
	s->sequence++;
	smp_wmb();
	s->sequence++;
}

inline int raw_read_seqcount_latch(seqcount_t *s)
{
    int seq = (*(volatile unsigned *)&(s->sequence));
	/* Pairs with the first smp_wmb() in raw_write_seqcount_latch() */
	/* smp_read_barrier_depends(); */
	return seq;
}

/**
 * raw_write_seqcount_latch - redirect readers to even/odd copy
 * @s: pointer to seqcount_t
 *
 * The latch technique is a multiversion concurrency control method that allows
 * queries during non-atomic modifications. If you can guarantee queries never
 * interrupt the modification -- e.g. the concurrency is strictly between CPUs
 * -- you most likely do not need this.
 *
 * Where the traditional RCU/lockless data structures rely on atomic
 * modifications to ensure queries observe either the old or the new state the
 * latch allows the same for non-atomic updates. The trade-off is doubling the
 * cost of storage; we have to maintain two copies of the entire data
 * structure.
 *
 * Very simply put: we first modify one copy and then the other. This ensures
 * there is always one copy in a stable state, ready to give us an answer.
 *
 * The basic form is a data structure like:
 *
 * struct latch_struct {
 *	seqcount_t		seq;
 *	struct data_struct	data[2];
 * };
 *
 * Where a modification, which is assumed to be externally serialized, does the
 * following:
 *
 * void latch_modify(struct latch_struct *latch, ...)
 * {
 *	smp_wmb();	<- Ensure that the last data[1] update is visible
 *	latch->seq++;
 *	smp_wmb();	<- Ensure that the seqcount update is visible
 *
 *	modify(latch->data[0], ...);
 *
 *	smp_wmb();	<- Ensure that the data[0] update is visible
 *	latch->seq++;
 *	smp_wmb();	<- Ensure that the seqcount update is visible
 *
 *	modify(latch->data[1], ...);
 * }
 *
 * The query will have a form like:
 *
 * struct entry *latch_query(struct latch_struct *latch, ...)
 * {
 *	struct entry *entry;
 *	unsigned seq, idx;
 *
 *	do {
 *		seq = raw_read_seqcount_latch(&latch->seq);
 *
 *		idx = seq & 0x01;
 *		entry = data_query(latch->data[idx], ...);
 *
 *		smp_rmb();
 *	} while (seq != latch->seq);
 *
 *	return entry;
 * }
 *
 * So during the modification, queries are first redirected to data[1]. Then we
 * modify data[0]. When that is complete, we redirect queries back to data[0]
 * and we can modify data[1].
 *
 * NOTE: The non-requirement for atomic modifications does _NOT_ include
 *       the publishing of new entries in the case where data is a dynamic
 *       data structure.
 *
 *       An iteration might start in data[0] and get suspended long enough
 *       to miss an entire modification sequence, once it resumes it might
 *       observe the new entry.
 *
 * NOTE: When data is a dynamic data structure; one should use regular RCU
 *       patterns to manage the lifetimes of the objects within.
 */
inline void raw_write_seqcount_latch(seqcount_t *s)
{
       smp_wmb();      /* prior stores before incrementing "sequence" */
       s->sequence++;
       smp_wmb();      /* increment "sequence" before following stores */
}

/*
 * Sequence counter only version assumes that callers are using their
 * own mutexing.
 */
inline void write_seqcount_begin_nested(seqcount_t *s, int subclass)
{
	raw_write_seqcount_begin(s);
}

inline void write_seqcount_begin(seqcount_t *s)
{
	write_seqcount_begin_nested(s, 0);
}

inline void write_seqcount_end(seqcount_t *s)
{
	raw_write_seqcount_end(s);
}

/**
 * write_seqcount_invalidate - invalidate in-progress read-side seq operations
 * @s: pointer to seqcount_t
 *
 * After write_seqcount_invalidate, no read-side seq operations will complete
 * successfully and see data older than this.
 */
inline void write_seqcount_invalidate(seqcount_t *s)
{
	smp_wmb();
	s->sequence+=2;
}

typedef struct {
	struct seqcount seqcount;
	spinlock_t lock;
} seqlock_t;

/*
 * Read side functions for starting and finalizing a read side section.
 */
inline unsigned read_seqbegin(const seqlock_t *sl)
{
	return read_seqcount_begin(&sl->seqcount);
}

inline unsigned read_seqretry(const seqlock_t *sl, unsigned start)
{
	return read_seqcount_retry(&sl->seqcount, start);
}

/*
 * Lock out other writers and update the count.
 * Acts like a normal spin_lock/unlock.
 * Don't need preempt_disable() because that is in the spin_lock already.
 */
inline void write_seqlock(seqlock_t *sl)
{
	spin_lock(&sl->lock);
	write_seqcount_begin(&sl->seqcount);
}

inline void write_sequnlock(seqlock_t *sl)
{
	write_seqcount_end(&sl->seqcount);
	spin_unlock(&sl->lock);
}

/*
 * A locking reader exclusively locks out other writers and locking readers,
 * but doesn't update the sequence number. Acts like a normal spin_lock/unlock.
 * Don't need preempt_disable() because that is in the spin_lock already.
 */
inline void read_seqlock_excl(seqlock_t *sl)
{
	spin_lock(&sl->lock);
}

inline void read_sequnlock_excl(seqlock_t *sl)
{
	spin_unlock(&sl->lock);
}

/**
 * read_seqbegin_or_lock - begin a sequence number check or locking block
 * @lock: sequence lock
 * @seq : sequence number to be checked
 *
 * First try it once optimistically without taking the lock. If that fails,
 * take the lock. The sequence number is also used as a marker for deciding
 * whether to be a reader (even) or writer (odd).
 * N.B. seq must be initialized to an even number to begin with.
 */
inline void read_seqbegin_or_lock(seqlock_t *lock, int *seq)
{
	if (!(*seq & 1))	/* Even */
		*seq = read_seqbegin(lock);
	else			/* Odd */
		read_seqlock_excl(lock);
}

inline int need_seqretry(seqlock_t *lock, int seq)
{
	return !(seq & 1) && read_seqretry(lock, seq);
}

inline void done_seqretry(seqlock_t *lock, int seq)
{
	if (seq & 1)
		read_sequnlock_excl(lock);
}
