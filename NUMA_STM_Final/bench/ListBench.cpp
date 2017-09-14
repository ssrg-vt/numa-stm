/**
 *  Copyright (C) 2011
 *  University of Rochester Department of Computer Science
 *    and
 *  Lehigh University Department of Computer Science and Engineering
 *
 * License: Modified BSD
 *          Please see the file LICENSE.RSTM for licensing information
 */

/**
 *  Step 1:
 *    Include the configuration code for the harness, and the API code.
 */

#include <iostream>
#include "../tm/rand_r_32.h"
#include "../tm/tm_thread.hpp"
#include <stdint.h>
/**
 *  We provide the option to build the entire benchmark in a single
 *  source. The bmconfig.hpp include defines all of the important functions
 *  that are implemented in this file, and bmharness.cpp defines the
 *  execution infrastructure.
 */

/**
 *  Step 2:
 *    Declare the data type that will be stress tested via this benchmark.
 *    Also provide any functions that will be needed to manipulate the data
 *    type.  Take care to avoid unnecessary indirection.
 */

#include "List.hpp"

#define LIST_SIZE 256
#define LOOKUP 40
#define INSERT 70
/**
 *  Step 3:
 *    Declare an instance of the data type, and provide init, test, and verify
 *    functions
 */

/*** the list we will manipulate in the experiment */
List* SET;

/*** Initialize the counter */
void bench_init()
{
    SET = new List();
    // warm up the datastructure
    //
    // NB: if we switch to CGL, we can initialize without transactions
    for (uint32_t w = 0; w < LIST_SIZE; w++) {
    	TM_BEGIN
    		SET->insert(w TM_PARAM);
    	TM_END
    }
}

/*** Run a bunch of increment transactions */
void bench_test(uintptr_t, uint32_t* seed)
{
    uint32_t val = rand_r_32(seed) % LIST_SIZE;
    uint32_t act = rand_r_32(seed) % 100;
    if (act < LOOKUP) {
        TM_BEGIN {
            SET->lookup(val TM_PARAM);
        } TM_END;
    }
    else if (act < INSERT) {
        TM_BEGIN {
            SET->insert(val TM_PARAM);
        } TM_END;
    }
    else {
        TM_BEGIN {
            SET->remove(val TM_PARAM);
        } TM_END;
    }
}

/*** Ensure the final state of the benchmark satisfies all invariants */
bool bench_verify() { return SET->isSane(); }

