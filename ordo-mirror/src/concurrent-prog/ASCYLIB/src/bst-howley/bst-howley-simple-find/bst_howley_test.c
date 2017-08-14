/*   
 *   File: bst_howley_test.c
 *   Author: Balmau Oana <oana.balmau@epfl.ch>, 
 *  	     Zablotchi Igor <igor.zablotchi@epfl.ch>,
 *   Description: 
 *   bst_howley_test.c is part of ASCYLIB
 *
 * Copyright (c) 2014 Vasileios Trigonakis <vasileios.trigonakis@epfl.ch>,
 * 	     	      Tudor David <tudor.david@epfl.ch>
 *	      	      Distributed Programming Lab (LPD), EPFL
 *
 * ASCYLIB is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, version 2
 * of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <assert.h>
#include <getopt.h>
#include <limits.h>
#include <signal.h>
#include <sys/time.h>
#include <time.h>


// #include "bst_howley.h"
#include "bst_howley.h"
#include "measurements.h"
#include "utils.h"
#include "atomic_ops_if.h"
#include "ssalloc.h"

int num_threads;
int op_count = 100;
uint8_t* v;

// TODO how do we know that threads are correctly set up on separate cpus?
// TODO seems to work correctly with num_threads = 10. WAT. wrap-around for set_cpu/the_cores?

//used to signal the threads when to stop
// ALIGNED(64) uint8_t running[64];

//the root of the binary search tree
node_t * root;

//a simple barrier implementation
//used to make sure all threads start the experiment at the same time
typedef struct barrier {
    pthread_cond_t complete;
    pthread_mutex_t mutex;
    int count;
    int crossing;
} barrier_t;

void barrier_init(barrier_t *b, int n)
{
    pthread_cond_init(&b->complete, NULL);
    pthread_mutex_init(&b->mutex, NULL);
    b->count = n;
    b->crossing = 0;
}

void barrier_cross(barrier_t *b)
{
    pthread_mutex_lock(&b->mutex);
    /* One more thread through */
    b->crossing++;
    /* If not all here, wait */
    if (b->crossing < b->count) {
        pthread_cond_wait(&b->complete, &b->mutex);
    } else {
        pthread_cond_broadcast(&b->complete);
        /* Reset for next time */
        b->crossing = 0;
    }
    pthread_mutex_unlock(&b->mutex);
}

//data structure through which we send parameters to and get results from the worker threads
typedef ALIGNED(64) struct thread_data {
    // pointer to the global barrier
    barrier_t *barrier;

    //counts the number of operations each thread performs
    // unsigned long num_operations;
    //total operation time (not used here)
    // ticks total_time;
    //seed (not used here)
    // unsigned int seed;
    //the number of elements each thread should add at the beginning of its execution
    // uint64_t num_add;
    //number of inserts a thread performs
    unsigned long num_insert;
    // number of removes a thread performs
    unsigned long num_remove;
    //number of searches a thread performs
    unsigned long num_search;
    //the id of the thread (used for thread placement on cores)
    int id;
} thread_data_t;

void *test(void *data) {
	fprintf(stderr, "Starting test\n");
	//get the per-thread data
    thread_data_t *d = (thread_data_t *)data;

    //place the thread on the apropriate cpu
    set_cpu(d->id);

    ssalloc_init();

    /* Wait on barrier */
    barrier_cross(d->barrier);

	int i;
	bool_t added;

	for ( i = 1; i <= op_count; i++){

		added = bst_add(i, root);
		// fprintf(stderr, "[%d] Added %d? %d\n", d->id, i, added==TRUE);
		if (added == TRUE) {
			d->num_insert++;
            FAI_U8(&v[i]);
		}
	}

    bst_print(root);

	// printf("Root right node: %d", root->right->key);
	
	for ( i = 1; i <= op_count; i++){

		bool_t found = bst_contains(i, root);
		// printf("Contains %d? %d\n", i, found==FOUND);
		if (found == FOUND) {
			d->num_search ++;
		} 
	}

	for ( i = 1; i <= op_count; i++){

		bool_t removed = bst_remove(i, root);
		// printf("Removed %d? %d\n", i, removed==TRUE);
		if (removed == TRUE) {
			d->num_remove ++;
            FAI_U8(&v[i]);
		}
	}

	// for ( i = 1; i < 10; i++){

	// 	bool_t found = bst_contains(i);
	// 	printf("Contains %d? %d\n", i, found==FOUND);
	// }


	return NULL;
}

int main(int argc, char* const argv[]) {

	num_threads = 1;
	int i;

    v = (uint8_t*) malloc((1+op_count) * sizeof(uint8_t));
    for (i = 1; i <= op_count;  i++) {
        v[i] = 0;
    }
    
	//place thread on the first cpu
    set_cpu(0);

    ssalloc_init();

    pthread_t *threads;
    pthread_attr_t attr;
    thread_data_t *data;
    barrier_t barrier;

	root = bst_initialize();
	printf("Initialized tree\n");

	//initialize the data which will be passed to the threads
    if ((data = (thread_data_t *)malloc(num_threads * sizeof(thread_data_t))) == NULL) {
        perror("malloc");
        exit(1);
    }

    if ((threads = (pthread_t *)malloc(num_threads * sizeof(pthread_t))) == NULL) {
        perror("malloc");
        exit(1);
    }

    //global barrier initialization (used to start the threads at the same time)
    barrier_init(&barrier, num_threads + 1);
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    for (i = 0; i < num_threads; i++) {

    	data[i].id = i;
        data[i].num_insert=0;
        data[i].num_remove=0;
        data[i].num_search=0;
        data[i].barrier = &barrier;


        if (pthread_create(&threads[i], &attr, test, (void *)(&data[i])) != 0) {
            fprintf(stderr, "Error creating thread\n");
            exit(1);
        }
    }
    pthread_attr_destroy(&attr);
    
    /* Start threads */
    barrier_cross(&barrier);

 	/* Wait for thread completion */
    for (i = 0; i < num_threads; i++) {
        if (pthread_join(threads[i], NULL) != 0) {
            fprintf(stderr, "Error waiting for thread completion\n");
            exit(1);
        }
    }

    for (i = 0; i < num_threads; i++) {
        printf("Thread %d\n", i);
   
        printf("  #inserts   : %lu\n", data[i].num_insert);
        printf("  #searches   : %lu\n", data[i].num_search);
        printf("  #removes   : %lu\n", data[i].num_remove);
        
    }

    bool_t correct = TRUE;
    for (i = 1; i <= op_count; i++) {
        if (v[i] != 2) {
            correct = FALSE;
            fprintf(stderr, "Incorrect value %d\n", i);
        }
    }
    if (correct == TRUE) {
        fprintf(stderr, "Okey-dokey\n");
    }
    fprintf(stderr, "%lu\n", bst_size(root));
    bst_print(root);

	free(threads);
    free(data);

	return 0;
}
