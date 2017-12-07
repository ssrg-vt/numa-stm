#ifndef TM_HPP
#define TM_HPP 1

#include <numa.h>
#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include <stdint.h>
#include <unistd.h>
#include <setjmp.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include "rand_r_32.h"
#include "WriteSet.hpp"
#include "locktable.hpp"


#ifdef DEBUG

#define LOG_SIZE 5000000
extern int logTM[LOG_SIZE];
extern int logiTM;

#endif

#define CLOCK_DIFF 378
//was 378
#define LOCKBIT  1

#define ZONES 4


#define FORCE_INLINE __attribute__((always_inline)) inline
#define CACHELINE_BYTES 64
#define CFENCE              __asm__ volatile ("":::"memory")
#define MFENCE  __asm__ volatile ("mfence":::"memory")


using stm::WriteSetEntry;
using stm::WriteSet;

struct pad_word_t
  {
      volatile uintptr_t val;
      char pad[CACHELINE_BYTES-sizeof(uintptr_t)];
  };

struct ts_vector
  {
      volatile uintptr_t val[ZONES];
  } __attribute__((aligned(CACHELINE_BYTES)));

extern ts_vector* ts_vectors[ZONES];
extern pad_word_t* ts_loc[ZONES];
extern lock_entry* lock_table[ZONES];

#define nop()       __asm__ volatile("nop")
#define pause()		__asm__ ( "pause;" )

#define NUM_STRIPES  1048576



inline unsigned long long get_real_time() {
	struct timespec time;
    clock_gettime(CLOCK_MONOTONIC_RAW, &time);

    return time.tv_sec * 1000000000L + time.tv_nsec;
}

inline long get_real_time_lo() {
	struct timespec time;
    clock_gettime(CLOCK_MONOTONIC_RAW, &time);

    return time.tv_nsec;
}

//inline uint64_t tick()
//{
//    uint32_t tmp[2];
//    __asm__ ("rdtsc" : "=a" (tmp[1]), "=d" (tmp[0]) : "c" (0x10) );
//    return (((uint64_t)tmp[0]) << 32) | tmp[1];
//}

//============================================================

#define PROCESS_NUM 8

//struct tm_obj {
//	//uint64_t owner;
//	uint64_t ver; //2 bit for state are used
//	uint64_t lock; //TODO make the lock part of the version //TODO make it volatile
//	volatile uint64_t flag; //TODO is volatile a must?
//	volatile uint64_t flag2; //TODO is volatile a must?
//	uintptr_t val;
//};

#define MAX_BATCH 8
#define BATCH_LIMIT 8

struct access_batch_msg {
	uint8_t batch_size;
	bool isUnlock;
	bool isWrite[MAX_BATCH];
	uint32_t ver[MAX_BATCH];
	uint64_t id[MAX_BATCH];
};

struct grant_batch_msg {
	uint8_t batch_size;
	uint64_t id[MAX_BATCH];
	bool isWrite[MAX_BATCH];
	bool granted[MAX_BATCH];
};

#define ACCESS_SIZE 102400
//Tx context
struct Tx_Context {
	int id;
	int numa_zone;
	jmp_buf scope;
	uintptr_t start_time[8];
	bool read_only;
	int reads_pos;
	int granted_reads_pos;
	uint64_t reads[ACCESS_SIZE];
	uint8_t r_zone[ACCESS_SIZE];
	bool granted_reads[ACCESS_SIZE];
	int writes_pos;
	int granted_writes_pos;
	uint64_t writes[ACCESS_SIZE];
	uint8_t w_zone[ACCESS_SIZE];
	int granted_writes[ACCESS_SIZE];
	WriteSet* writeset;
	bool touched_zones[8];
	long commits =0, aborts =0, count=0, internuma =0, mineNotChanged=0;
	int backoff;
	unsigned int seed;
//	pad_word_t* thread_locks[500];
#ifdef STATISTICS
#endif
};

extern __thread Tx_Context* Self;

#define COMMIT_RSLT 1
#define ABORT_RSLT 2

struct pad_msg_t
  {
      volatile uintptr_t ready;
      volatile uintptr_t result;
      Tx_Context* tx;
      char pad[CACHELINE_BYTES-(sizeof(uintptr_t)*2)-sizeof(Tx_Context*)];
  } __attribute__((aligned(CACHELINE_BYTES)));

extern pad_msg_t* comm_channel[ZONES];

//extern pad_word_t* thread_locks[500];

extern pad_word_t counter;

FORCE_INLINE
uint64_t read_tsc(void)
{
//	uint32_t a, d;
//	__asm __volatile("rdtsc" : "=a" (a), "=d" (d));
//	return ((uint64_t) a) | (((uint64_t) d) << 32);
	return __sync_fetch_and_add(&(counter.val), 1);
}


#define TM_TX_VAR	Tx_Context* tx = (Tx_Context*)Self;

#define TM_ARG , Tx_Context* tx
#define TM_ARG_ALONE Tx_Context* tx
#define TM_PARAM ,tx


#define TM_FREE(a)  /*nothing for now*/

#define TM_ALLOC(a, b) numa_malloc(a, b)

FORCE_INLINE void tm_abort(Tx_Context* tx, int explicitly);

FORCE_INLINE void printlogTM(int processId) {
#ifdef DEBUG
	for (int i=0; i<logiTM; i+=9) {
		if (logTM[i+7] == 1) { //received
			printf("%d: (t=%d) response from %d regarding %s object %d for %s is %s (version: %d)\n", processId, logTM[i+8], logTM[i], logTM[i+1]? "lock": "unlock",
					logTM[i+2], logTM[i+3]? "write" : "read", logTM[i+5]? "granted" : "not granted", logTM[i+4]);
		} else if (logTM[i+7] == 2) { //batch sent
			printf("%d: (t=%d) batch sent to %d of size %d for %s\n", processId, logTM[i+8], logTM[i], logTM[i+2], logTM[i+1]? "lock": "unlock");
		} else if (logTM[i+7] == 3) { //abort
			printf("%d: (t=%d) aborted\n", processId, logTM[i+8]);
		} else if (logTM[i+7] == 4) { //commit
			printf("%d: (t=%d) committed\n", processId, logTM[i+8]);
		} else if (logTM[i+7] == 5) { //begin
			printf("%d: (t=%d) begin tx\n", processId, logTM[i+8]);
		} else if (logTM[i+6]) { //local
			//printf("%d: zone %d locally %s object %d for %s (version: %d)\n", processId, logTM[i], logTM[i+1]? "locked": "unlocked",
				//	logTM[i+2], logTM[i+3]? "write" : "read", logTM[i+4]);
			printf("%d: (t=%d) zone %d locally do %s of object %d for %s and is %s (version: %d)\n", processId, logTM[i+8], logTM[i], logTM[i+1]? "lock": "unlock",
							logTM[i+2], logTM[i+3]? "write" : "read", logTM[i+5] ==1? "granted": (logTM[i+5] ==2? "upgraded":(logTM[i+5] ==3? "shared": "not granted")), logTM[i+4]);
		} else {
			printf("%d: (t=%d) request to %d to %s object %d for %s (version: %d)\n", processId, logTM[i+8], logTM[i], logTM[i+1]? "lock": "unlock",
					logTM[i+2], logTM[i+3]? "write" : "read", logTM[i+4]);
		}
	}
	logiTM = 0;
#endif
}

FORCE_INLINE void addToLogTM(int serverId, int objId, int isWrite, int isLock, int version, int isGranted, int isLocal, int operation) {
#ifdef DEBUG
	logTM[logiTM++] = serverId;
	logTM[logiTM++] = isLock;
	logTM[logiTM++] = objId;
	logTM[logiTM++] = isWrite;
	logTM[logiTM++] = version;
	logTM[logiTM++] = isGranted;
	logTM[logiTM++] = isLocal;
	logTM[logiTM++] = operation;
#ifdef DEBUG_W_TIME
	logTM[logiTM++] = get_real_time_lo();
#else
	logTM[logiTM++] = 0;
#endif

	if (logiTM + 9 >= LOG_SIZE) {
		printf("Log exhausted, flushing...\n");
		printlogTM(Self->id);
	}
#endif
}


template <typename T>
FORCE_INLINE T tm_read(tm_obj<T>* addr, Tx_Context* tx, int numa_zone)
{
    WriteSetEntry log((void**)addr);
    bool found = tx->writeset->find(log);
    if (__builtin_expect(found, false))
		return *((T*) (&log.val.i64));


    tm_obj<T> * obj = (tm_obj<T> *) addr;

	uint64_t v1 = obj->ver;
	CFENCE;
	T val = obj->val;
	CFENCE;
	uint64_t v2 = obj->ver;
	if (v1 > tx->start_time[0] || (v1 != v2) || (*(obj->lock_p) > 0)) {
		//printf("error in read l=%d, v1=%lld, v2=%lld, start=%lld\n", *(obj->lock_p), v1, v2,tx->start_time[0]);
		tm_abort(tx, 0);
	}
	int r_pos = tx->reads_pos++;
	tx->reads[r_pos] = addr;
	return val;
}

template <typename T>
FORCE_INLINE void tm_write(tm_obj<T>* addr, T val, uint8_t size, Tx_Context* tx, int numa_zone)
{
    bool alreadyExists = tx->writeset->insert(WriteSetEntry((void**)addr, *((uint64_t*)(&val)), size));
    if (!alreadyExists) {//TOOD use the writeset to lock!!
		int w_pos = tx->writes_pos++;
		tx->writes[w_pos] = addr;//reinterpret_cast<uint64_t>(addr)>>3) % TABLE_SIZE;
//		tx->w_zone[w_pos] = numa_zone;
//    	tx->touched_zones[numa_zone] = true;
		tx->granted_writes[w_pos] = 0;
    }
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
      static void write(tm_obj<T>* addr, T val, Tx_Context* tx, int numa_zone)
      {
          InvalidTypeAsSecondTemplateParameter itaftp;
          T invalid = (T)itaftp;
      }
  };


template <typename T>
struct DISPATCH<T, 1>
{
	FORCE_INLINE
    static void write(tm_obj<T>* addr, T val, Tx_Context* tx, int numa_zone)
    {
		tm_write(addr, val, 1, tx, numa_zone);
    }
};
template <typename T>
struct DISPATCH<T, 2>
{
	FORCE_INLINE
    static void write(tm_obj<T>* addr, T val, Tx_Context* tx, int numa_zone)
    {
		tm_write(addr, val, 2, tx, numa_zone);
    }
};

  template <typename T>
  struct DISPATCH<T, 4>
  {
	  FORCE_INLINE
      static void write(tm_obj<T>* addr, T val, Tx_Context* tx, int numa_zone)
      {
		  tm_write(addr, val, 4, tx, numa_zone);
      }
  };

  template <typename T>
  struct DISPATCH<T, 8>
  {
	  FORCE_INLINE
      static void write(tm_obj<T>* addr, T val, Tx_Context* tx, int numa_zone)
      {
		  tm_write(addr, val, 8, tx, numa_zone);
      }
  };


template <typename T>
FORCE_INLINE void tm_write(tm_obj<T>* addr, T val, Tx_Context* tx, int numa_zone)
{
	DISPATCH<T, sizeof(T)>::write(addr, val, tx, numa_zone);
}


#define TM_READ(var)       tm_read(&var, tx, tx->numa_zone)
#define TM_WRITE(var, val) tm_write(&var, val, tx, tx->numa_zone)


#define TM_READ_Z(var, numa_zone)       tm_read(&var, tx, numa_zone)
#define TM_WRITE_Z(var, val, numa_zone) tm_write(&var, val, tx, numa_zone)



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

FORCE_INLINE void thread_end() {
	Tx_Context* tx = (Tx_Context*)Self;
	printf("%d: commits = %d, aborts = %d, my zone %d, out of zone = %d\n", tx->id, tx->commits, tx->aborts, tx->numa_zone, tx->internuma);
}

FORCE_INLINE void thread_init(int id, int numa_zone, int index) {
	if (!Self) {
		Self = new Tx_Context();
		Tx_Context* tx = (Tx_Context*)Self;
		tx->id = id;//__sync_fetch_and_add(&tm_thread_count, 1);
		tx->seed = tx->id;
		tx->numa_zone = numa_zone;
		//mpass_create_message_end_point(tx->id, PROCESS_COUNT);
		tx->writeset = new WriteSet(ACCESS_SIZE);
//		printf("tx pointer %d\n", tx);
//		tx->undo_log = (AddrVal*) malloc(10000*sizeof(AddrVal));
//		//prefetch memory
//		for (int j=0; j<10000; j++) {
//			tx->undo_log[j].addr = (void **) 0;
//			tx->undo_log[j].val.i64 = 0;
//			tx->undo_log[j].size = 0;
//		}
		for (int j=0; j<ACCESS_SIZE; j++) {
			tx->granted_reads[j] = false;
			tx->granted_writes[j] = 0;
		}
		for (int i=0; i<ZONES;i++) {
			tx->start_time[i] = 0;
		}
//		thread_locks[id] = (pad_word_t*) numa_alloc_onnode(sizeof(pad_word_t), numa_zone);
//		thread_locks[id]->val = 0;
//		if (id == 0) {
//			thread_locks[400] = (pad_word_t*) numa_alloc_onnode(sizeof(pad_word_t), numa_zone);
//			thread_locks[400]->val = 0;
//		}
//		for (int i=0; i < SRV_COUNT; i++)
//			lock_table[i] = get_lock_table(i);

//		if (id == 0) {
//
//		} else {
//			for (int i=0; i<8;i++) {
//				ts_vectors[i] = get_shared_mem(i, sizeof(ts_vector), SHARED_MEM_KEY1);
//				ts_loc[i] = get_shared_mem(i, sizeof(ts_vector), SHARED_MEM_KEY2);
//				//tx->timestamps[i] = get_shared_mem(i, sizeof(pad_word_t));
//			}
//		}
	}
	else {
//		printf("init local th lock pointers\n");
		Tx_Context* tx = (Tx_Context*)Self;
//		for (int i=0; i<500; i++) {
//			tx->thread_locks[i] = thread_locks[i];
//		}
	}
}

FORCE_INLINE uintptr_t tm_srv_commit(Tx_Context* tx){}

FORCE_INLINE void tm_sys_init() {
	printf("ORDO!");
	for (int i=0; i<ZONES;i++) {
		lock_table[i] = numa_alloc_onnode(sizeof(lock_entry) * TABLE_SIZE, i);//create_lock_table(i);
		ts_vectors[i] = numa_alloc_onnode(sizeof(ts_vector), i); //create_shared_mem(i, sizeof(ts_vector), SHARED_MEM_KEY1);
		ts_loc[i] = numa_alloc_onnode(sizeof(ts_vector), i);//create_shared_mem(i, sizeof(ts_vector), SHARED_MEM_KEY2);
		ts_loc[i]->val = 0;
		for (int j=0; j<ZONES; j++)
			ts_vectors[i]->val[j] = 1;
		//tx->timestamps[i] = create_shared_mem(i, sizeof(pad_word_t));
	}
}


FORCE_INLINE void tm_abort(Tx_Context* tx, int explicitly)
{
	tx->aborts++;

	//exp_backoff(tx);

	//restart the tx
    longjmp(tx->scope, 1);
}

FORCE_INLINE void tm_commit(Tx_Context* tx)
{
	if (tx->writeset->size() == 0) { //read-only
		tx->commits++;
		return;
	}

	//TODO make threads id start from 1
	int idP1 = tx->id + 1;

	bool failed = false;
	volatile int* volatile firstLock;
	for (int i = 0; i < tx->writes_pos; i++) {
		tm_obj<uint8_t>* obj = (tm_obj<uint8_t>*) tx->writes[i];
		if (i == 0) {
			firstLock = obj->lock_p;
		}
		volatile int* volatile tmpLock = obj->lock_p;

		if (*(obj->lock_p) == idP1) {
			tx->granted_writes[i] = 2;
		} else if (__sync_bool_compare_and_swap(tmpLock, 0, idP1)) {
			if (tmpLock != obj->lock_p) {
				*tmpLock = 0;
//				printf("found!\n");
				failed = true;
				break;
			} else {
				tx->granted_writes[i] = 1;
			}
		} else {
			failed = true;
			break;
		}
	}

	if (failed) { //unlock and abort
		for (int i = 0; i < tx->writes_pos; i++) {
			tm_obj<uint8_t>* obj = (tm_obj<uint8_t>*) tx->writes[i];
			if (tx->granted_writes[i] == 1) {
				*(obj->lock_p) = 0;
			} else if (!tx->granted_writes[i]){
				break;
			}
		}
//		printf("error in commit1");
		tm_abort(tx, 0);
	}

	//TODO check lock also
 	bool do_abort = false;
	//validate reads
	for (int i = 0; i < tx->reads_pos; i++) {
		tm_obj<uint8_t>* obj = (tm_obj<uint8_t>*) tx->reads[i];
		if (obj->ver > tx->start_time[0]) {
			do_abort = true;
			break;
		}
	}

	if (do_abort) {
		//TODO May be we can keep the ownership
		for (int i = 0; i < tx->writes_pos; i++) {
			tm_obj<uint8_t>* obj = (tm_obj<uint8_t>*) tx->writes[i];
			if (tx->granted_writes[i] == 1) {
				*(obj->lock_p) = 0;
			}
//			else {
//				break;
//			}
		}
//		printf("error in commit2");
		tm_abort(tx, 0);
	}

	tx->writeset->writeback();

	uintptr_t next_ts = read_tsc();// << LOCKBIT;//ts_vectors[0]->val[0]+1;//__sync_val_compare_and_swap(&ts_vectors[0]->val[0], cur_ts, cur_ts+1);//__sync_fetch_and_add(&ts_vectors[0]->val[0], 1);

//	CFENCE;
//	if (next_ts - tx->start_time[0] < CLOCK_DIFF) {
//		int i = (CLOCK_DIFF - (next_ts - tx->start_time[0])) / 10;
//		while (i-- >= 0) {
//			asm volatile ("pause");
//		}
//		next_ts = read_tsc();// << LOCKBIT;
//	}

	//update versions & unlock
//	int* lock = (int*) malloc(sizeof(int));
//	*lock = 1;

	for (int i = 0; i < tx->writes_pos; i++) {
		tm_obj<uint8_t>* obj = (tm_obj<uint8_t>*) tx->writes[i];
		obj->ver = next_ts;
//		volatile int* volatile oldP = obj->lock_p;
//		CFENCE;
//		obj->lock_p = firstLock;
//		MFENCE;
		volatile int* volatile oldP = obj->lock_p;
//		MFENCE;
//		obj->lock_p = lock;
		obj->lock_p = firstLock;
//		MFENCE;
//		*(oldP) = 0;
//		*lock = 0;
//		MFENCE;
//		MFENCE;
//		obj->lock_p = oldP;
//		obj->lock = 0;
//		__sync_bool_compare_and_swap(&obj->lock_p, 0, idP1)
		if (oldP != firstLock)
			*(oldP) = 0;
	}
//	printf("%d %d \n", tx->writes_pos, *lock);
//	MFENCE;
//	CFENCE;
	*(firstLock) = 0;

	tx->commits++;
}

#define TM_BEGIN2												\
	{															\
		Tx_Context* tx = (Tx_Context*)Self;          			\
		tx->backoff=-1;											\
		uint32_t abort_flags = _setjmp (tx->scope);				\
		{														\
			addToLogTM(0, 0, 0, 0, 0, 0, 0, 5);					\
			tx->backoff++;										\
			tx->read_only = true;								\
			tx->reads_pos =0;									\
			tx->granted_reads_pos =0;							\
			tx->writes_pos =0;									\
			tx->granted_writes_pos =0;							\
			tx->writeset->reset();								\
			tx->count++;		\
			tx->start_time[0] = read_tsc();


//<< LOCKBIT;

/*tx->count++;		\
			if (tx->count % 10 == 0) { \
				for (int i=0; i<SRV_COUNT; i++) {					\
					tx->start_time[i] = tx->timestamps[i]->val;							\
				}	\
			} else { \
				tx->start_time[tx->numa_zone] = tx->timestamps[tx->numa_zone]->val; \
			}
*/

//			for (int i=0; i<SRV_COUNT; i++) {
//				tx->batch_msg[i].batch_size = 0;
//			}


#define TM_END2                                  	\
			tm_commit(tx);                          \
		}											\
	}

//============MEMORY==========================================

//extern char* tm_mem_buffer;
//extern pad_word_t tm_mem_pos;

//inline void tm_memory_init(int numThread) {
//	int buffer_size = numThread * 100 * 1024 * 1024;
//	tm_mem_buffer = (char*) malloc(buffer_size); //100 MB for each thread
//	//prefetch all these pages
//	for (int i=buffer_size-1; i >=0 ; i--) {
//		tm_mem_buffer[i] = 0;
//	}
//	tm_mem_pos.pad[0] = numThread;
//}
//
//inline void tm_memory_free() {
//	free(tm_mem_buffer);
//}
//
//FORCE_INLINE void* tm_malloc(int size) {
//	uintptr_t pos = __sync_fetch_and_add(&tm_mem_pos.val, size);
//	if (pos >= 100 * 1024 * 1024 * tm_mem_pos.pad[0]) {
//		printf("ERROR, memory buffer exhausted!!!! %d\n", pos);
//		exit(1);
//	}
//	return (void*) (tm_mem_buffer + pos);
//}
//============================================================


#endif //TM_HPP
