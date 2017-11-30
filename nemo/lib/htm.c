#include "htm.hpp"


pad_word_t timestamp = {0};
pad_word_t last_init     = {0};
pad_word_t last_complete = {0};

filter_t   ring_wf[RING_ELEMENTS] __attribute__((aligned(16)));

filter_t write_locks;

//int RETRY_SHORT = 1;

__thread Tx_Context* Self;

pad_word_t active_tx = {0};
pad_word_t tm_super_glock = {0};

volatile int tm_thread_count = 1;

char* tm_mem_buffer;
pad_word_t tm_mem_pos = {0};


//NOT part of our library but used in STAMP

long int    FALSE = 0,
    TRUE  = 1;

//int SUPER_GL_LIMIT = 1;
//int SUPER_GL_LIMIT2 = 1;
