#ifndef HTM_HPP
#define HTM_HPP 1


#include "tsx-assert.h"
#include "rtm.h"
#include "rand_r_32.h"
#include "stm/BitFilter.hpp"
#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include <stdint.h>
#include <unistd.h>
#include <setjmp.h>

#define FORCE_INLINE __attribute__((always_inline)) inline
#define CACHELINE_BYTES 64
#define CFENCE              __asm__ volatile ("":::"memory")

using stm::BitFilter;

struct pad_word_t
  {
      volatile uintptr_t val;
      char pad[CACHELINE_BYTES-sizeof(uintptr_t)];
  };

struct AddrVal
  {
      void** addr;
      union {
    	  uint8_t i8;
    	  uint16_t i16;
    	  uint32_t i32;
    	  uint64_t i64;
      } val;
      char size;
  };

//BLOOM_SIZE is define in build time
typedef BitFilter<4096> filter_t;
static const uint32_t RING_ELEMENTS = 1024;

extern pad_word_t timestamp;
extern pad_word_t last_init;
extern pad_word_t last_complete;

extern filter_t   ring_wf[RING_ELEMENTS] __attribute__((aligned(16)));



#define nop()       __asm__ volatile("nop")
#define pause()		__asm__ ( "pause;" )

#define NUM_STRIPES  1048576

inline unsigned long long get_real_time() {
	struct timespec time;
    clock_gettime(CLOCK_MONOTONIC_RAW, &time);

    return time.tv_sec * 1000000000L + time.tv_nsec;
}


#define X_CONFLICT 1
#define X_GL_EXISTS  2
#define X_EXPLICIT_EXT  3

extern filter_t write_locks;

//============================================================
//It is now part of the build system
//#define STATISTICS
//extern int RETRY_SHORT;

extern volatile int tm_thread_count;
//Tx context
struct Tx_Context {
	int id;
	filter_t rf;
	filter_t wf;
	filter_t p_wf;
	AddrVal* undo_log;
	int undo_i;
	jmp_buf scope;
	int nesting_depth = 0;
	int backoff;
	uintptr_t start_time;
	bool read_only;
	bool go_super_gl = false;
	unsigned int seed;
#ifdef STATISTICS
	int conflict_abort = 0, capacity_abort = 0, other_abort = 0, our_conflict_abort = 0, external_abort = 0;
	int htm_conflict_abort = 0, htm_a_conflict_abort =0, htm_capacity_abort = 0, htm_other_abort = 0, htm_success=0;
	int sw_c = 0, sw_abort=0;
	int gl_c = 0;
#endif
};

//It is now part of the build system
//#define RETRY_SHORT		5

#define SUPER_GL

#ifdef SUPER_GL

//It is now part of the build system
//#define SUPER_GL_LIMIT 		1000
//#define SUPER_GL_LIMIT2		1000

extern pad_word_t active_tx;
extern pad_word_t tm_super_glock;

#define SUPER_GL_SHORT_CHK \
	if (tm_super_glock.val) _xabort(X_GL_EXISTS);

#define SUPER_GL_ACQUIRE 													\
 	if (tm_super_glock.val != tx->id) while (tm_super_glock.val) pause();	\
	if (tx->go_super_gl && tm_super_glock.val != tx->id) {					\
		/*printf("about to acquire\n");*/\
		do {																\
			while (tm_super_glock.val) pause();								\
		} while (!__sync_bool_compare_and_swap(&tm_super_glock.val, 0, tx->id));\
		while(active_tx.val) pause();										\
		/*printf("lock acquired\n");*/\
		GL_STATISTICS														\
	}																		\
	__sync_fetch_and_add(&active_tx.val, 1);


#define SUPER_GL_INIT int other_abort_num = 0, notOther_abort_num = 0;

#define SUPER_GL_TX_BEGIN \
		if (tx->go_super_gl || (status = _xbegin()) == _XBEGIN_STARTED) {

#define SUPER_GL_CHECK \
	if (!tx->go_super_gl) {

#define SUPER_GL_CHECK_END }

#define SUPER_GL_COND \
	if (!status) other_abort_num++; else notOther_abort_num++;											\
	if (other_abort_num > SUPER_GL_LIMIT || notOther_abort_num > SUPER_GL_LIMIT2) {						\
		/*printf("abort due to others is high %d %c\n", tm_super_glock.val, tx->go_super_gl? 't':'f');*/\
		tx->go_super_gl = true;															\
		tm_abort(tx, 0);																\
	}

#define SUPER_GL_TX_EXIST \
	__sync_fetch_and_add(&active_tx.val, -1);


#define SUPER_GL_UNLOCK \
	if (tx->go_super_gl) {						\
		tx->go_super_gl = false;				\
		tm_super_glock.val = 0;				\
	}										\
	SUPER_GL_TX_EXIST



#else
#define SUPER_GL_SHORT_CHK
#define SUPER_GL_ACQUIRE
#define SUPER_GL_INIT
#define SUPER_GL_TX_BEGIN \
		if ((status = _xbegin()) == _XBEGIN_STARTED) {

#define SUPER_GL_CHECK
#define SUPER_GL_CHECK_END
#define SUPER_GL_COND
#define SUPER_GL_UNLOCK
#define SUPER_GL_TX_EXIST
#endif


extern __thread Tx_Context* Self;

#define TM_TX_VAR	Tx_Context* tx = (Tx_Context*)Self;

template <typename T>
FORCE_INLINE T tm_read(T* addr, Tx_Context* tx)
{
	SUPER_GL_CHECK
	tx->rf.add(addr);
	SUPER_GL_CHECK_END
    return *addr;//reinterpret_cast<T>(*addr);
}

template <typename T>
FORCE_INLINE void tm_short_write(T* addr, T val, Tx_Context* tx)
{
	tx->wf.add(addr);
    *addr = val;
}


template <typename T>
FORCE_INLINE void tm_write(T* addr, T val, char size, Tx_Context* tx)
{
	tx->undo_log[tx->undo_i].addr = (void **) addr;
	tx->undo_log[tx->undo_i].val.i64 = *(reinterpret_cast<uint64_t*>(addr));
	tx->undo_log[tx->undo_i].size = size;
	tx->undo_i++;
	tx->p_wf.add(addr);
    *addr = val;
}


template <typename T, size_t S>
  struct DISPATCH
  {
      // use this to ensure compile-time errors
      struct InvalidTypeAsSecondTemplateParameter { };

      // the read method will transform a read to a sizeof(T) byte range
      // starting at addr into a set of stmread_word calls.  For now, the
      // range must be aligned on a sizeof(T) boundary, and T must be 1, 4,
      // or 8 bytes.
      FORCE_INLINE
      static void write(T* addr, T val, Tx_Context* tx)
      {
          InvalidTypeAsSecondTemplateParameter itaftp;
          T invalid = (T)itaftp;
      }
  };


template <typename T>
struct DISPATCH<T, 1>
{
	FORCE_INLINE
    static void write(T* addr, T val, Tx_Context* tx)
    {
		tm_write(addr, val, 1, tx);
    }
};
template <typename T>
struct DISPATCH<T, 2>
{
	FORCE_INLINE
    static void write(T* addr, T val, Tx_Context* tx)
    {
		tm_write(addr, val, 2, tx);
    }
};

  template <typename T>
  struct DISPATCH<T, 4>
  {
	  FORCE_INLINE
      static void write(T* addr, T val, Tx_Context* tx)
      {
		  tm_write(addr, val, 4, tx);
      }
  };

  template <typename T>
  struct DISPATCH<T, 8>
  {
	  FORCE_INLINE
      static void write(T* addr, T val, Tx_Context* tx)
      {
		  tm_write(addr, val, 8, tx);
      }
  };


template <typename T>
FORCE_INLINE void tm_write(T* addr, T val, Tx_Context* tx)
{
	SUPER_GL_CHECK
	DISPATCH<T, sizeof(T)>::write(addr, val, tx);
	return;
	SUPER_GL_CHECK_END
	*addr = val;
}


#define TM_READ(var)       tm_read(&var, tx)
#define TM_SHORT_WRITE(var, val) tm_short_write(&var, val, tx)
#define TM_WRITE(var, val) tm_write(&var, val, tx)

FORCE_INLINE void spin64() {
	for (int i = 0; i < 64; i++)
		nop();
}

static const uint32_t BACKOFF_MIN   = 4;        // min backoff exponent
static const uint32_t BACKOFF_MAX   = 6;//16;       // max backoff exponent

FORCE_INLINE void exp_backoff(Tx_Context* tx)
{
  // how many bits should we use to pick an amount of time to wait?
  uint32_t bits = tx->backoff + BACKOFF_MIN - 1;
  bits = (bits > BACKOFF_MAX) ? BACKOFF_MAX : bits;
  // get a random amount of time to wait, bounded by an exponentially
  // increasing limit
  int32_t delay = rand_r_32(&tx->seed);
  delay &= ((1 << bits)-1);
  // wait until at least that many ns have passed
  unsigned long long start = get_real_time();
  unsigned long long stop_at = start + delay;
  while (get_real_time() < stop_at) { spin64(); }
//  for (int i=0; i<delay;i++) spin64();
}


FORCE_INLINE void thread_init() {
	if (!Self) {
		Self = new Tx_Context();
		Tx_Context* tx = (Tx_Context*)Self;
		tx->id = __sync_fetch_and_add(&tm_thread_count, 1);
		tx->seed = tx->id;
//		printf("tx pointer %d\n", tx);
		tx->undo_log = (AddrVal*) malloc(10000*sizeof(AddrVal));
		//prefetch memory
		for (int j=0; j<10000; j++) {
			tx->undo_log[j].addr = (void **) 0;
			tx->undo_log[j].val.i64 = 0;
			tx->undo_log[j].size = 0;
		}
	}
}



FORCE_INLINE void tm_abort(Tx_Context* tx, int explicitly)
{
	//if we are in an HTM transaction we must first abort it. Otherwise, this instruction will be treated as nop
	_xabort(X_EXPLICIT_EXT);
#ifdef SUPER_GL
	if (explicitly && tm_super_glock.val == tx->id) {
		return;
	}
#endif

	if (!tx->read_only){
	  while (tx->undo_i > 0) { //TODO if size matter, we need to keep the size also or check how RSTM did it
		  tx->undo_i--;
		  switch(tx->undo_log[tx->undo_i].size) {
		  	  case 1:
		  		*((uint8_t*)tx->undo_log[tx->undo_i].addr) = tx->undo_log[tx->undo_i].val.i8;
			  break;
		  	  case 2:
		  		*((uint16_t*)tx->undo_log[tx->undo_i].addr) = tx->undo_log[tx->undo_i].val.i16;
			  break;
		  	  case 4:
		  		*((uint32_t*)tx->undo_log[tx->undo_i].addr) = tx->undo_log[tx->undo_i].val.i32;
			  break;
		  	  case 8:
		  		*((uint64_t*)tx->undo_log[tx->undo_i].addr) = tx->undo_log[tx->undo_i].val.i64;
			  break;
		  }

	  }

	  write_locks.differencewith(tx->wf);
	  tx->wf.clear();
	}
	tx->rf.clear();

//	for (int i = 0; i < 64*tx->backoff; i++)
//		nop();
	exp_backoff(tx);

#ifdef STATISTICS
	tx->sw_abort++;
#endif
	SUPER_GL_TX_EXIST

	//if (!explicitly)
	//restart the tx
    longjmp(tx->scope, 1);
}

FORCE_INLINE void tm_commit(Tx_Context* tx)
{
	if (tx->read_only) {
		tx->rf.clear();
#ifdef STATISTICS
		tx->sw_c++;
#endif
	} else {
		uintptr_t commit_time;
		do {
			  commit_time = timestamp.val;
			  // get the latest ring entry, return if we've seen it already
			  if (commit_time != tx->start_time) {
				  // wait for the latest entry to be initialized
				  while (last_init.val < commit_time)
					  spin64();

				  // intersect against all new entries
				  for (uintptr_t i = commit_time; i >= tx->start_time + 1; i--)
					  if (ring_wf[i % RING_ELEMENTS].intersect(&tx->rf)) {
						  tm_abort(tx, 0);
					  }

				  // wait for newest entry to be wb-complete before continuing
				  while (last_complete.val < commit_time)
					  spin64();

				  // detect ring rollover: start.ts must not have changed
				  if (timestamp.val > (tx->start_time + RING_ELEMENTS)) {
					  tm_abort(tx, 0);
				  }

				  // ensure this tx doesn't look at this entry again
				  tx->start_time = commit_time;
			  }
		} while (!__sync_bool_compare_and_swap(&timestamp.val, commit_time, commit_time + 1));

		// copy the bits over (use SSE, not indirection)
		ring_wf[(commit_time + 1) % RING_ELEMENTS].fastcopy(&tx->wf);

		// setting this says "the bits are valid"
		last_init.val = commit_time + 1;
		//write through so mark ring entry COMPLETE
		last_complete.val = commit_time + 1;
#ifdef STATISTICS
		tx->sw_c++;
#endif
		write_locks.differencewith(tx->wf);
		CFENCE;
		tx->rf.clear();
		tx->wf.clear();
	}
}

FORCE_INLINE void tm_check_partition(Tx_Context* tx)
{
	//TODO Do partial validation "check_in_flight" (will need partial read signature also)
	uintptr_t my_index = last_init.val;
	if (__builtin_expect(my_index != tx->start_time, true)) { //TODO is false correct!?
		  // intersect against all new entries
		  for (uintptr_t i = my_index; i >= tx->start_time + 1; i--)
			  if (ring_wf[i % RING_ELEMENTS].intersect(&tx->rf)) { //full abort
				  tm_abort(tx, 0);
			  }
		  // wait for newest entry to be writeback-complete before returning
		  while (last_complete.val < my_index)
			  spin64();

		  // detect ring rollover: start.ts must not have changed
		  if (timestamp.val > (tx->start_time + RING_ELEMENTS)) {
			  tm_abort(tx, 0);
		  }

		  // ensure this tx doesn't look at this entry again
		  tx->start_time = my_index;
	}
}


#ifdef	STATISTICS
#define ALL_HTM_STAT1		tx->htm_success++;

#define ALL_HTM_STAT2															\
	if (status & _XABORT_CODE(X_CONFLICT)) {									\
		tx->htm_a_conflict_abort++;												\
	} else if (status & _XABORT_CAPACITY)										\
		tx->htm_capacity_abort++;												\
	else if (status & _XABORT_CONFLICT){										\
		tx->htm_conflict_abort++;												\
	}																			\
	else																		\
		tx->htm_other_abort++;

#define PART_HTM_STAT_1 	tx->our_conflict_abort++;

#define PART_HTM_STAT_2		tx->external_abort++;

#define PART_HTM_STAT_3															\
	if (status & _XABORT_CAPACITY) 												\
		tx->capacity_abort++; 													\
	else if (status & _XABORT_CONFLICT) 										\
		tx->conflict_abort++; 													\
	else 																		\
		tx->other_abort++;

#define GL_STATISTICS	 tx->gl_c++;

#else
#define ALL_HTM_STAT1
#define ALL_HTM_STAT2
#define PART_HTM_STAT_1
#define PART_HTM_STAT_2
#define PART_HTM_STAT_3
#define GL_STATISTICS
#endif

//USE THIS ONLY TO TEST YOUR FRAMEWORK QUICKLY WITH NO SHORT TX SUPPORT (NOT FOR REAL RESULST
#define TM_NO_SHORT_BEGIN									\
	{														\
	Tx_Context* tx = (Tx_Context*)Self;          			\
	unsigned status;										\
	int htm_retry = 0;										\
	bool htm_again;											\
	do {													\
		htm_again = false;									\
		{


#define TM_SHORT_BEGIN										\
	{														\
	Tx_Context* tx = (Tx_Context*)Self;          			\
	unsigned status;										\
	int htm_retry = 0;										\
	bool htm_again;											\
	do {													\
		htm_again = false;									\
		if ((status = _xbegin()) == _XBEGIN_STARTED) {		\



#define TM_SHORT_END																\
		if (tx->rf.intersect(&write_locks) || tx->wf.intersect(&write_locks)) {		\
			_xabort(X_CONFLICT);													\
		}																			\
		SUPER_GL_SHORT_CHK															\
		_xend();																	\
		tx->rf.clear();																\
		tx->wf.clear();																\
		ALL_HTM_STAT1																\
	} else {																		\
		ALL_HTM_STAT2																\
		htm_retry++;																\
		if (htm_retry < RETRY_SHORT) {												\
			htm_again = true;														\
			continue;																\
		}


//#define GL_1ST_LEVEL


#ifdef GL_1ST_LEVEL

#define PART_RETRY_LIMIT 10
#define TM_RETRY_LIMIT 10

volatile int tm_glock = 0;

#define GL_1ST_LEVEL_VAR	\
	int part_retry = 0;		\
	int tm_retry = 0;		\
	bool lock_owner = false;

#define GL_1ST_LEVEL_BLOCK												\
	if (!lock_owner) {													\
		while (tm_glock) pause();										\
		tm_retry++;														\
		if (tm_retry > TM_RETRY_LIMIT) {								\
			do {														\
				while (tm_glock) pause();								\
			} while (!__sync_bool_compare_and_swap(&tm_glock, 0, 1));	\
			lock_owner = true;											\
		}																\
	}

#define GL_1ST_LEVEL_PRT_INIT	part_retry = 0;

#define GL_1ST_LEVEL_PRT_CHECK										\
	part_retry++;													\
	if (part_retry > PART_RETRY_LIMIT) {							\
		tm_abort(tx, 0);												\
	}

#define GL_1ST_LEVEL_UNLCK				\
	if (lock_owner) {					\
		lock_owner = false;				\
		tm_glock = 0;					\
    }									\

#else

#define GL_1ST_LEVEL_BLOCK
#define GL_1ST_LEVEL_VAR
#define GL_1ST_LEVEL_PRT_INIT
#define GL_1ST_LEVEL_PRT_CHECK
#define GL_1ST_LEVEL_UNLCK
#endif


#define TM_GL_BEGIN		                                	\
	GL_1ST_LEVEL_VAR										\
	tx->go_super_gl = true;									\
	tx->backoff=0;											\
	uint32_t abort_flags = _setjmp (tx->scope);				\
	{														\
		SUPER_GL_ACQUIRE 									\
		tx->read_only = true;								\
		tx->undo_i =0;										\
		GL_1ST_LEVEL_BLOCK									\
		tx->start_time = last_complete.val;



#define TM_LONG_BEGIN		                                \
	GL_1ST_LEVEL_VAR										\
	tx->go_super_gl = false;								\
	tx->backoff=0;											\
	uint32_t abort_flags = _setjmp (tx->scope);				\
	{														\
		SUPER_GL_ACQUIRE 									\
		tx->read_only = true;								\
		tx->undo_i =0;										\
		GL_1ST_LEVEL_BLOCK									\
		tx->start_time = last_complete.val;


#define TM_PART_BEGIN		                                \
{															\
	SUPER_GL_INIT											\
	GL_1ST_LEVEL_PRT_INIT									\
	bool more = true;										\
	unsigned status;										\
	while (more) {		                                    \
		tx->backoff++;										\
		GL_1ST_LEVEL_PRT_CHECK								\
		more = false;	                                    \
		tx->p_wf.clear();	                                \
		SUPER_GL_TX_BEGIN


//TODO partial abort and check if complete abort is necessary when xabort is called
//TODO Do partial validation "check_in_flight" (will need partial read signature also)
//TODO is false correct in built in expect!?
//TODO Do partial abort with xabort?
#define TM_PART_END					                                									\
		SUPER_GL_CHECK																					\
		if (tx->rf.intersect(&write_locks, tx->wf) || tx->p_wf.intersect(&write_locks, tx->wf)) {		\
			_xabort(X_CONFLICT);					                                					\
		}								                                								\
		else if (tx->undo_i){							                                				\
			tx->read_only = false;														\
			write_locks.unionwith(tx->p_wf);					                        \
		}					                                							\
		_xend();					                                					\
		tx->wf.unionwith(tx->p_wf);			                                			\
		tm_check_partition(tx);															\
		SUPER_GL_CHECK_END																\
	} else {																			\
		if (status & _XABORT_CODE(X_CONFLICT)) { 										\
			PART_HTM_STAT_1																\
			tm_abort(tx, 0);															\
		} else if (status & _XABORT_CODE(X_EXPLICIT_EXT)) {								\
			PART_HTM_STAT_2																\
			tm_abort(tx, 1);															\
		} else 																			\
			more = true; 																\
		SUPER_GL_COND																	\
		PART_HTM_STAT_3			 														\
		/*for (int i = 0; i < 64*tx->backoff; i++)*/									\
			/*nop();*/exp_backoff(tx);													\
	}																					\
  }																						\
}


#define TM_LONG_END                                  		\
				SUPER_GL_CHECK							\
				tm_commit(tx);                          \
				SUPER_GL_CHECK_END						\
				GL_1ST_LEVEL_UNLCK						\
				SUPER_GL_UNLOCK							\
			  }											\
			}	                                  		\
		} while (htm_again);							\
	}
//============MEMORY==========================================

extern char* tm_mem_buffer;
extern pad_word_t tm_mem_pos;

inline void tm_memory_init(int numThread) {
	int buffer_size = numThread * 100 * 1024 * 1024;
	tm_mem_buffer = (char*) malloc(buffer_size); //100 MB for each thread
	//prefetch all these pages
	for (int i=buffer_size-1; i >=0 ; i--) {
		tm_mem_buffer[i] = 0;
	}
	tm_mem_pos.pad[0] = numThread;
}

inline void tm_memory_free() {
	free(tm_mem_buffer);
}

FORCE_INLINE void* tm_malloc(int size) {
	uintptr_t pos = __sync_fetch_and_add(&tm_mem_pos.val, size);
	if (pos >= 100 * 1024 * 1024 * tm_mem_pos.pad[0]) {
		printf("ERROR, memory buffer exhausted!!!! %d\n", pos);
		exit(1);
	}
	return (void*) (tm_mem_buffer + pos);
}
//============================================================
//Example fo
/*

void *tx_fn(void *arg) {

	thread_init();

	int notRetried = 0;

	unsigned int seed = (long)arg;
	int threadId = seed;
	int tx_limit = 0;
	int loc[1024];

	while(barrier);


	unsigned long long time = get_real_time();
	bool do_gl_test = true;

	while (tx_limit < NO_TX) {

		int tx_len = RANDOMIZE? (rand_r_32(&seed) % NO_READWRITE) + 1 : NO_READWRITE;
		tx_limit++;
		for (int i = 0; i < tx_len; ++i) {
//			loc[i] = tx_len*seed + i;//rand_r_32(&seed) % ARRAY_SIZE;
			loc[i] = rand_r_32(&seed) % ARRAY_SIZE;
		}

//		TM_SHORT_BEGIN
//				for (int i= 0; i < tx_len; i+=2) {
//					if (TM_READ(array[loc[i]]) > 50) {
//						TM_SHORT_WRITE(array[loc[i+1]], array[loc[i+1]] + 50);
//						TM_SHORT_WRITE(array[loc[i]], array[loc[i]] - 50);
//					}
//				}
//		TM_SHORT_END
			//failed in all HTM, retry with partitioning
		TM_NO_SHORT_BEGIN

		TM_LONG_BEGIN

				int initial = 0;
				do{
					int limit = initial + SPLIT_SIZE > tx_len? tx_len: initial + SPLIT_SIZE;
					TM_PART_BEGIN
						if (do_gl_test) {
							do_gl_test = false;
							malloc(100000);
//							printf("in=====================================\n");
						}
						int i;
						for (i=initial; i < limit; i+=2) {
							if (TM_READ(array[loc[i]]) > 50) {
								TM_WRITE(array[loc[i+1]], array[loc[i+1]] + 50);
								TM_WRITE(array[loc[i]], array[loc[i]] - 50);
							}
						}
					TM_PART_END
					initial = limit;
				} while (initial!=tx_len);
		TM_LONG_END
	} //num of tx loop
	time = get_real_time() - time;
	printf("Time = %llu\n", time);
#ifdef	STATISTICS
	TM_TX_VAR
	printf("conflict = %d, capacity=%d, other=%d, our conflict=%d, external=%d\n", tx->conflict_abort, tx->capacity_abort, tx->other_abort, tx->our_conflict_abort, tx->external_abort);
	printf("HTM conflict = %d, our conflict = %d, capacity=%d, other=%d\n", tx->htm_conflict_abort, tx->htm_a_conflict_abort, tx->htm_capacity_abort, tx->htm_other_abort);
	printf("Last complete = %d, SW count = %d, HTM count = %d, abort count = %d\n", last_complete.val, tx->sw_c, tx->htm_success, tx->sw_abort);
	printf("global locking = %d\n", tx->gl_c);
#endif
	printf("seed final = %d\n", seed);
	write_locks.print();
}


int main_disabled(int argc, char* argv[])
{
	if (argc < 4) {
		printf("Usage #_of_randomly_accessed_element  Array_size #_of_short_tx_retrials\n");
		exit(0);
	}
	NO_READWRITE = atoi(argv[1]);
	ARRAY_SIZE = atoi(argv[2]);
	RETRY_SHORT = atoi(argv[3]);

//	char* huge_buffer = (char*) malloc(5000000);
//
//	huge_buffer += 100;
//
//	array = (long*) huge_buffer;
	array = (long*) malloc(ARRAY_SIZE * sizeof(long));
//	printf("%d %d\n", sizeof(int), sizeof(long));
	//prefetch all memory pages
	long oo=0;
	for (int i = 0 ; i< ARRAY_SIZE; i++) {
		array[i]=1000;
	}
        for (int i = 0 ; i< ARRAY_SIZE; i++) {
                oo += array[i];
        }
        printf("Inital sum = %d\n", oo);


//    	AddrVal* undo_log;
//    	undo_log = (AddrVal*) malloc(10000*sizeof(AddrVal));
//    	int undo_i=0;
//
//		undo_log[undo_i].addr = (void **) &(array[0]);
//		undo_log[undo_i].val.val = (void *) (array[0]);
//		undo_i++;
//		undo_log[undo_i].addr = (void **) &(array[1]);
//		undo_log[undo_i].val.val = (void *) (array[1]);
//		undo_i++;
//		undo_log[undo_i].addr = (void **) &(array[10]);
//		undo_log[undo_i].val.val = (void *) (array[10]);
//		undo_i++;
//
//
//		  while (undo_i > 0) { //TODO if size matter, we need to keep the size also or check how RSTM did it
//				  undo_i--;
//				  printf("o = %d\n", undo_log[undo_i].val);
//				  (*((int*)undo_log[undo_i].addr)) = undo_log[undo_i].val.i;
////							  *(undo_log[undo_i-2]) = undo_log[undo_i-1];
////							  undo_i -= 2;
//			  }
//		  for (int i =0; i<20 ; i++) {
//			  printf("%d =  %d \n", i, array[i]);
//		  }


  pthread_t thread1, thread2, thread3, thread4;
  pthread_attr_t thread_attr;
  pthread_attr_init(&thread_attr);

	unsigned status;
//	int x[100000] = {1};
	int i;

	pthread_create(&thread1, &thread_attr, tx_fn, (void*)45678);
	pthread_create(&thread2, &thread_attr, tx_fn, (void*)4968147);
	pthread_create(&thread3, &thread_attr, tx_fn, (void*)49147);
	pthread_create(&thread4, &thread_attr, tx_fn, (void*)4967);

//	pthread_create(&thread1, &thread_attr, tx_fn, (void*)1);
//	pthread_create(&thread2, &thread_attr, tx_fn, (void*)2);
//	pthread_create(&thread3, &thread_attr, tx_fn, (void*)3);
//	pthread_create(&thread4, &thread_attr, tx_fn, (void*)4);


	unsigned long long time = get_real_time();
	barrier =0;

	pthread_join(thread1, NULL);
	pthread_join(thread2, NULL);
	pthread_join(thread3, NULL);
	pthread_join(thread4, NULL);
	time = get_real_time() - time;
	printf("Total Time = %llu\n", time);
	oo=0;        
	for (int i = 0 ; i< ARRAY_SIZE; i++) {
                oo += array[i];
//                printf("I%d = %d  ", i, array[i]);
//                if (i%10 == 0)
//                	printf("\n");
        }
	printf("Final sum = %d\n", oo);

	return 0;
}

*/

#endif //HTM_HPP
