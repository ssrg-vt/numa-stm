#ifndef TM_HPP
#define TM_HPP 1

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
#include <errno.h>
#include <string.h>
#include <errno.h>
#include "rand_r_32.h"
#include "WriteSet.hpp"
#include "locktable.hpp"


#ifdef DEBUG

#define LOG_SIZE 5000000
extern int logTM[LOG_SIZE];
extern int logiTM;

#endif


#define FORCE_INLINE __attribute__((always_inline)) inline
#define CACHELINE_BYTES 64
#define CFENCE              __asm__ volatile ("":::"memory")


using stm::WriteSetEntry;
using stm::WriteSet;

struct pad_word_t
  {
      volatile uintptr_t val;
      char pad[CACHELINE_BYTES-sizeof(uintptr_t)];
  };

struct ts_vector
  {
      volatile uintptr_t val[8];
  } __attribute__((aligned(CACHELINE_BYTES)));

extern ts_vector* ts_vectors[8];
extern pad_word_t* ts_loc[8];
extern lock_entry* lock_table[8];

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

inline uint64_t tick()
{
    uint32_t tmp[2];
    __asm__ ("rdtsc" : "=a" (tmp[1]), "=d" (tmp[0]) : "c" (0x10) );
    return (((uint64_t)tmp[0]) << 32) | tmp[1];
}

//============================================================

#define PROCESS_NUM 8

struct tm_obj {
	uint64_t id;
	uint64_t ver;
};

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
	bool granted_writes[ACCESS_SIZE];
	WriteSet* writeset;
	//uint8_t touched_zones[8];
	long commits =0, aborts =0, count=0, internuma =0, mineNotChanged=0;
	int backoff;
	unsigned int seed;
#ifdef STATISTICS
#endif
};

extern __thread Tx_Context* Self;

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

#define ZONES 8

template <typename T>
FORCE_INLINE T tm_read(T* addr, Tx_Context* tx, int numa_zone)
{
    WriteSetEntry log((void**)addr);
    bool found = tx->writeset->find(log);
    if (__builtin_expect(found, false))
		return (T) log.val.i64;

	uint64_t index = (reinterpret_cast<uint64_t>(addr)>>3) % TABLE_SIZE;
	lock_entry* entry_p = &(lock_table[numa_zone][index]);


	bool again = true;
	while(again) {
		again = false;
		lock_entry entry = *entry_p;
		if (!(entry.lock_owner & LOCK_MASK)) { //not locked
			if (!__sync_bool_compare_and_swap(&(entry_p->lock_owner), entry.lock_owner, (tx->id << 4) | READ_LOCK)) {
				again = true;
				continue;
			}
		} else if ((entry.lock_owner & LOCK_MASK) == READ_LOCK) {
			if ((entry.lock_owner & READ_LOCK_MULTI_OWNERS) == READ_LOCK_MULTI_OWNERS) {
				if (!__sync_bool_compare_and_swap(&(entry_p->lock_owner), entry.lock_owner, entry.lock_owner + ONE_MASK)) {
					again = true;
					continue;
				}
			} else if ((entry.lock_owner >> 4) != tx->id){
				if (!__sync_bool_compare_and_swap(&(entry_p->lock_owner), entry.lock_owner, READ_LOCK_MULTI_OWNERS_INIT)) {
					again = true;
					continue;
				}
			}
		} else if ((entry.lock_owner >> 4) != tx->id) {
			tm_abort(tx, 0);
		}
	}
    //tx->touched_zones[numa_zone] |= 1;
	int r_pos = tx->reads_pos++;
	tx->reads[r_pos] = index;
	tx->r_zone[r_pos] = numa_zone;
	tx->granted_reads[r_pos] = true;
	return *addr;
}

template <typename T>
FORCE_INLINE void tm_write(T* addr, T val, char size, Tx_Context* tx, int numa_zone)
{
    bool alreadyExists = tx->writeset->insert(WriteSetEntry((void**)addr, (uint64_t)val, size));
    if (!alreadyExists) {//TOOD use the writeset to lock!!
    	uint64_t index = (reinterpret_cast<uint64_t>(addr)>>3) % TABLE_SIZE;
    	lock_entry* entry_p = &(lock_table[numa_zone][index]);

		bool again = true;
		while(again) {
			again = false;
			lock_entry entry = *entry_p;
			if (!(entry.lock_owner & LOCK_MASK)) { //not locked
				if (!__sync_bool_compare_and_swap(&(entry_p->lock_owner), entry.lock_owner, (tx->id << 4) | WRITE_LOCK)) {
					again = true;
					continue;
				}
			} else if ((entry.lock_owner & LOCK_MASK) == READ_LOCK) {
				if ((entry.lock_owner >> 4) == tx->id) {
					if (!__sync_bool_compare_and_swap(&(entry_p->lock_owner), entry.lock_owner, entry.lock_owner | WRITE_LOCK)) {
						again = true;
						continue;
					}
				} else {
					tm_abort(tx, 0);
				}
			} else if ((entry.lock_owner >> 4) != tx->id){
				tm_abort(tx, 0);
			}
		}
    	int w_pos = tx->writes_pos++;
		tx->writes[w_pos] = index;
		tx->w_zone[w_pos] = numa_zone;
    	//tx->touched_zones[numa_zone] = 2;
		tx->granted_writes[w_pos] = true;
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
      static void write(T* addr, T val, Tx_Context* tx, int numa_zone)
      {
          InvalidTypeAsSecondTemplateParameter itaftp;
          T invalid = (T)itaftp;
      }
  };


template <typename T>
struct DISPATCH<T, 1>
{
	FORCE_INLINE
    static void write(T* addr, T val, Tx_Context* tx, int numa_zone)
    {
		tm_write(addr, val, 1, tx, numa_zone);
    }
};
template <typename T>
struct DISPATCH<T, 2>
{
	FORCE_INLINE
    static void write(T* addr, T val, Tx_Context* tx, int numa_zone)
    {
		tm_write(addr, val, 2, tx, numa_zone);
    }
};

  template <typename T>
  struct DISPATCH<T, 4>
  {
	  FORCE_INLINE
      static void write(T* addr, T val, Tx_Context* tx, int numa_zone)
      {
		  tm_write(addr, val, 4, tx, numa_zone);
      }
  };

  template <typename T>
  struct DISPATCH<T, 8>
  {
	  FORCE_INLINE
      static void write(T* addr, T val, Tx_Context* tx, int numa_zone)
      {
		  tm_write(addr, val, 8, tx, numa_zone);
      }
  };


template <typename T>
FORCE_INLINE void tm_write(T* addr, T val, Tx_Context* tx, int numa_zone)
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


FORCE_INLINE void thread_init(int id, int numa_zone) {
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
			tx->granted_writes[j] = false;
		}

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
}

FORCE_INLINE void tm_sys_init() {
	for (int i=0; i<8;i++) {
		lock_table[i] = numa_alloc_onnode(sizeof(lock_entry) * TABLE_SIZE, i);//create_lock_table(i);
		memset (lock_table[i], 0, sizeof(lock_entry) * TABLE_SIZE);
		ts_vectors[i] = numa_alloc_onnode(sizeof(ts_vector), i); //create_shared_mem(i, sizeof(ts_vector), SHARED_MEM_KEY1);
		ts_loc[i] = numa_alloc_onnode(sizeof(ts_vector), i);//create_shared_mem(i, sizeof(ts_vector), SHARED_MEM_KEY2);
		ts_loc[i]->val = 0;
		for (int j=0; j<8; j++)
			ts_vectors[i]->val[j] = 1;
		//tx->timestamps[i] = create_shared_mem(i, sizeof(pad_word_t));
	}
}


FORCE_INLINE void tm_abort(Tx_Context* tx, int explicitly)
{

	addToLogTM(0, 0, 0, 0, 0, 0, 0, 3);
	//release locks
	for (int i = 0; i < tx->reads_pos; i++) {
		if (tx->granted_reads[i]) {
			lock_entry* entry_p = &(lock_table[tx->r_zone[i]][tx->reads[i]]);
			bool again = true;
			while(again) {
				again = false;
				lock_entry entry = *entry_p;
				if ((entry.lock_owner & LOCK_MASK) == READ_LOCK) {
					if (tx->id == (entry.lock_owner >> 4)) {
						if (!__sync_bool_compare_and_swap(&(entry_p->lock_owner), entry.lock_owner, entry.lock_owner & UNLOCK_MASK)) {
							again = true;
							continue;
						}
					} else if ((entry.lock_owner & READ_LOCK_MULTI_OWNERS) == READ_LOCK_MULTI_OWNERS) {
						if (entry.lock_owner == READ_LOCK_MULTI_OWNERS_ONE) {
							if (!__sync_bool_compare_and_swap(&(entry_p->lock_owner), entry.lock_owner, entry.lock_owner & UNLOCK_MASK)) {
								again = true;
								continue;
							}
						} else {
							if (!__sync_bool_compare_and_swap(&(entry_p->lock_owner), entry.lock_owner, entry.lock_owner - ONE_MASK)) {
								again = true;
								continue;
							}
						}
					} else {
						//printf("Something really weird, in unlock a read lock!!\n");
					}
				}
			}
		}
	}

	for (int i = 0; i < tx->writes_pos; i++) {
		if (tx->granted_writes[i]) {
			lock_entry* entry_p = &(lock_table[tx->w_zone[i]][tx->writes[i]]);
			bool again = true;
			while(again) {
				again = false;
				lock_entry entry = *entry_p;
				if ((entry.lock_owner & LOCK_MASK) == WRITE_LOCK && tx->id == (entry.lock_owner >> 4)) {
					if (!__sync_bool_compare_and_swap(&(entry_p->lock_owner), entry.lock_owner, entry.lock_owner & UNLOCK_MASK)) {
						again = true;
						continue;
					}
				}
			}
		}
	}

	tx->aborts++;

	//exp_backoff(tx);

	//restart the tx
    longjmp(tx->scope, 1);
}

FORCE_INLINE void tm_commit(Tx_Context* tx)
{
	tx->writeset->writeback();

	//release locks
	for (int i = 0; i < tx->reads_pos; i++) {
		if (tx->granted_reads[i]) {
			int srv_addr = tx->r_zone[i];
			lock_entry* entry_p = &(lock_table[srv_addr][tx->reads[i]]);
			bool again = true;
			while(again) {
				again = false;
				lock_entry entry = *entry_p;
				if ((entry.lock_owner & LOCK_MASK) == READ_LOCK) {
					if (tx->id == (entry.lock_owner >> 4)) {
						if (!__sync_bool_compare_and_swap(&(entry_p->lock_owner), entry.lock_owner, entry.lock_owner & UNLOCK_MASK)) {
							again = true;
							continue;
						}
					} else if ((entry.lock_owner & READ_LOCK_MULTI_OWNERS) == READ_LOCK_MULTI_OWNERS) {
						if (entry.lock_owner == READ_LOCK_MULTI_OWNERS_ONE) {
							if (!__sync_bool_compare_and_swap(&(entry_p->lock_owner), entry.lock_owner, entry.lock_owner & UNLOCK_MASK)) {
								again = true;
								continue;
							}
						} else {
							if (!__sync_bool_compare_and_swap(&(entry_p->lock_owner), entry.lock_owner, entry.lock_owner - ONE_MASK)) {
								again = true;
								continue;
							}
						}
					} else {
						//printf("Something really weird, in unlock a read lock!!\n");
					}
				}
			}
		}
	}

	for (int i = 0; i < tx->writes_pos; i++) {
		if (tx->granted_writes[i]) {
			int srv_addr = tx->w_zone[i];
			lock_entry* entry_p = &(lock_table[srv_addr][tx->writes[i]]);
			bool again = true;
			while(again) {
				again = false;
				lock_entry entry = *entry_p;
				if ((entry.lock_owner & LOCK_MASK) == WRITE_LOCK && tx->id == (entry.lock_owner >> 4)) {
					if (!__sync_bool_compare_and_swap(&(entry_p->lock_owner), entry.lock_owner, entry.lock_owner & UNLOCK_MASK)) {
						again = true;
						continue;
					}
				}
			}
		}
	}

	addToLogTM(0, 0, 0, 0, 0, 0, 0, 4);
	tx->commits++;
}


#define TM_BEGIN												\
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



//tx->start_time[tx->numa_zone] = ts_vectors[tx->numa_zone]->val[tx->numa_zone];

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


#define TM_END                                  	\
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
