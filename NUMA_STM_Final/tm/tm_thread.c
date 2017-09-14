#include "tm_thread.hpp"
#include "numa_lock.hpp"

//pad_word_t* timestamp;
//pad_word_t last_init     = {0};
//pad_word_t last_complete = {0};

__thread Tx_Context* Self;

pad_word_t active_tx = {0};
//pad_word_t tm_super_glock = {0};

//volatile int tm_thread_count = 8;

//char* tm_mem_buffer;
//pad_word_t tm_mem_pos = {0};
//NOT part of our library but used in STAMP

long int    FALSE = 0,
    TRUE  = 1;

ts_vector* ts_vectors[ZONES];
pad_word_t* ts_loc[ZONES];
lock_entry* lock_table[ZONES];
pad_msg_t* comm_channel[ZONES];

Numa_Lock numa_lock;

#ifdef DEBUG

int logTM[LOG_SIZE];
int logiTM = 0;

#endif

//TODO get zone thread count also
void* server_run(void * args)
{
    int numa_zone = (long) args;

    while (true) {
    	for (int i=0; i<50; i++) {
    		if (comm_channel[numa_zone][i].ready) {
    			comm_channel[numa_zone][i].ready = 0;
    			comm_channel[numa_zone][i].result = tm_srv_commit(comm_channel[numa_zone][i].tx);
    		}
    	}
    }
}

//int SUPER_GL_LIMIT = 1;
//int SUPER_GL_LIMIT2 = 1;
