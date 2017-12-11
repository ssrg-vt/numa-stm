/* =============================================================================
 *
 * heap.c
 *
 * =============================================================================
 *
 * Copyright (C) Stanford University, 2006.  All Rights Reserved.
 * Author: Chi Cao Minh
 *
 * =============================================================================
 *
 * For the license of bayes/sort.h and bayes/sort.c, please see the header
 * of the files.
 * 
 * ------------------------------------------------------------------------
 * 
 * For the license of kmeans, please see kmeans/LICENSE.kmeans
 * 
 * ------------------------------------------------------------------------
 * 
 * For the license of ssca2, please see ssca2/COPYRIGHT
 * 
 * ------------------------------------------------------------------------
 * 
 * For the license of lib/mt19937ar.c and lib/mt19937ar.h, please see the
 * header of the files.
 * 
 * ------------------------------------------------------------------------
 * 
 * For the license of lib/rbtree.h and lib/rbtree.c, please see
 * lib/LEGALNOTICE.rbtree and lib/LICENSE.rbtree
 * 
 * ------------------------------------------------------------------------
 * 
 * Unless otherwise noted, the following license applies to STAMP files:
 * 
 * Copyright (c) 2007, Stanford University
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 * 
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 * 
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in
 *       the documentation and/or other materials provided with the
 *       distribution.
 * 
 *     * Neither the name of Stanford University nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY STANFORD UNIVERSITY ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL STANFORD UNIVERSITY BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 *
 * =============================================================================
 */


#include <stdlib.h>
#include <assert.h>
#include "heap.h"
#include "tm.h"
#include "types.h"

struct heap {
	tm_obj<tm_obj<void*>*> elements;
	tm_obj<long> size;
	tm_obj<long> capacity;
    long (*compare)(const void*, const void*);
};


#define PARENT(i)       ((i) / 2)
#define LEFT_CHILD(i)   (2*i)
#define RIGHT_CHILD(i)  (2*(i) + 1)

/* =============================================================================
 * DECLARATION OF TM_CALLABLE FUNCTIONS
 * =============================================================================
 */

TM_CALLABLE
static void
TMsiftUp (TM_ARGDECL  heap_t* heapPtr, long startIndex);

TM_CALLABLE
static void
TMheapify (TM_ARGDECL  heap_t* heapPtr, long startIndex);


/* =============================================================================
 * heap_alloc
 * -- Returns NULL on failure
 * =============================================================================
 */
heap_t*
heap_alloc (long initCapacity, long (*compare)(const void*, const void*))
{
    heap_t* heapPtr;

    heapPtr = (heap_t*)malloc(sizeof(heap_t));
    if (heapPtr) {
        long capacity = ((initCapacity > 0) ? (initCapacity) : (1));
        heapPtr->elements.val = (tm_obj<void*>*)malloc(capacity * sizeof(tm_obj<void*>));
        assert(heapPtr->elements.val);
        heapPtr->elements.lock_p = &(heapPtr->elements.lock);
        heapPtr->elements.lock = 0;
        heapPtr->elements.ver = 0;
        memset(heapPtr->elements.val, 0, capacity * sizeof(tm_obj<void*>));
        for (int i=0; i < capacity; i++) {
        	heapPtr->elements.val[i].lock_p = &(heapPtr->elements.val[i].lock);
        }
        heapPtr->size.val = 0;
        heapPtr->size.lock_p = &(heapPtr->size.lock);
        heapPtr->size.lock = 0;
        heapPtr->size.ver = 0;
        heapPtr->capacity.val = capacity;
        heapPtr->capacity.lock_p = &(heapPtr->capacity.lock);
        heapPtr->capacity.lock = 0;
        heapPtr->capacity.ver = 0;
        heapPtr->compare = compare;
    }

    return heapPtr;
}


/* =============================================================================
 * heap_free
 * =============================================================================
 */
void
heap_free (heap_t* heapPtr)
{
    free(heapPtr->elements.val);
    free(heapPtr);
}


/* =============================================================================
 * siftUp
 * =============================================================================
 */
static void
siftUp (heap_t* heapPtr, long startIndex)
{
    tm_obj<void*>* elements = heapPtr->elements.val;
    long (*compare)(const void*, const void*) = heapPtr->compare;

    long index = startIndex;
    while ((index > 1)) {
        long parentIndex = PARENT(index);
        void* parentPtr = elements[parentIndex].val;
        void* thisPtr   = elements[index].val;
        if (compare(parentPtr, thisPtr) >= 0) {
            break;
        }
        void* tmpPtr = parentPtr;
        elements[parentIndex].val = thisPtr;
        elements[index].val = tmpPtr;
        index = parentIndex;
    }
}


/* =============================================================================
 * TMsiftUp
 * =============================================================================
 */
static void
TMsiftUp (TM_ARGDECL  heap_t* heapPtr, long startIndex)
{
	tm_obj<void*>* elements = (tm_obj<void*>*)TM_SHARED_READ_P(heapPtr->elements);
    long (*compare)(const void*, const void*) = heapPtr->compare;

    long index = startIndex;
    while ((index > 1)) {
        long parentIndex = PARENT(index);
        void* parentPtr = (void*)TM_SHARED_READ_P(elements[parentIndex]);
        void* thisPtr   = (void*)TM_SHARED_READ_P(elements[index]);
        if (compare(parentPtr, thisPtr) >= 0) {
            break;
        }
        void* tmpPtr = parentPtr;
        TM_SHARED_WRITE_P(elements[parentIndex], thisPtr);
        TM_SHARED_WRITE_P(elements[index], tmpPtr);
        index = parentIndex;
    }
}


/* =============================================================================
 * heap_insert
 * -- Returns FALSE on failure
 * =============================================================================
 */
bool_t
heap_insert (heap_t* heapPtr, void* dataPtr)
{
    long size = heapPtr->size.val;
    long capacity = heapPtr->capacity.val;

    if ((size + 1) >= capacity) {
        long newCapacity = capacity * 2;
        tm_obj<void*>* newElements = (tm_obj<void*>*)malloc(newCapacity * sizeof(tm_obj<void*>));
        if (newElements == NULL) {
            return FALSE;
        }
        memset(newElements, 0, newCapacity * sizeof(tm_obj<void*>));
		for (int i=0; i < newCapacity; i++) {
			newElements[i].lock_p = &(newElements[i].lock);
		}
        heapPtr->capacity.val = newCapacity;
        long i;
        tm_obj<void*>* elements = heapPtr->elements.val;
        for (i = 0; i <= size; i++) {
            newElements[i].val = elements[i].val;
        }
        free(heapPtr->elements.val);
        heapPtr->elements.val = newElements;
    }

    size = ++(heapPtr->size.val);
    heapPtr->elements.val[size].val = dataPtr;
    siftUp(heapPtr, size);

    return TRUE;
}


/* =============================================================================
 * TMheap_insert
 * -- Returns FALSE on failure
 * =============================================================================
 */
bool_t
TMheap_insert (TM_ARGDECL  heap_t* heapPtr, void* dataPtr)
{
    long size = (long)TM_SHARED_READ(heapPtr->size);
    long capacity = (long)TM_SHARED_READ(heapPtr->capacity);

    if ((size + 1) >= capacity) {
        long newCapacity = capacity * 2;
        tm_obj<void*>* newElements = (tm_obj<void*>*)malloc(newCapacity * sizeof(tm_obj<void*>));
        if (newElements == NULL) {
            return FALSE;
        }
        memset(newElements, 0, newCapacity * sizeof(tm_obj<void*>));
		for (int i=0; i < newCapacity; i++) {
			newElements[i].lock_p = &(newElements[i].lock);
		}
        TM_SHARED_WRITE(heapPtr->capacity, newCapacity);
        long i;
        tm_obj<void*>* elements = TM_SHARED_READ_P(heapPtr->elements);
        for (i = 0; i <= size; i++) {
            newElements[i].val = (void*)TM_SHARED_READ_P(elements[i]);
        }
        TM_FREE(heapPtr->elements.val);
        TM_SHARED_WRITE(heapPtr->elements, newElements);
    }

    size++;
    TM_SHARED_WRITE(heapPtr->size, size);
    tm_obj<void*>* elements = (tm_obj<void*>*)TM_SHARED_READ_P(heapPtr->elements);
    TM_SHARED_WRITE_P(elements[size], dataPtr);
    TMsiftUp(TM_ARG  heapPtr, size);

    return TRUE;
}


/* =============================================================================
 * heapify
 * =============================================================================
 */
static void
heapify (heap_t* heapPtr, long startIndex)
{
	tm_obj<void*>* elements = heapPtr->elements.val;
    long (*compare)(const void*, const void*) = heapPtr->compare;

    long size = heapPtr->size.val;
    long index = startIndex;

    while (1) {

        long leftIndex = LEFT_CHILD(index);
        long rightIndex = RIGHT_CHILD(index);
        long maxIndex = -1;

        if ((leftIndex <= size) &&
            (compare(elements[leftIndex].val, elements[index].val) > 0))
        {
            maxIndex = leftIndex;
        } else {
            maxIndex = index;
        }

        if ((rightIndex <= size) &&
            (compare(elements[rightIndex].val, elements[maxIndex].val) > 0))
        {
            maxIndex = rightIndex;
        }

        if (maxIndex == index) {
            break;
        } else {
            void* tmpPtr = elements[index].val;
            elements[index].val = elements[maxIndex].val;
            elements[maxIndex].val = tmpPtr;
            index = maxIndex;
        }
    }
}


/* =============================================================================
 * TMheapify
 * =============================================================================
 */
static void
TMheapify (TM_ARGDECL  heap_t* heapPtr, long startIndex)
{
	tm_obj<void*>* elements = (tm_obj<void*>*)TM_SHARED_READ_P(heapPtr->elements);
    long (*compare)(const void*, const void*) = heapPtr->compare;

    long size = (long)TM_SHARED_READ(heapPtr->size);
    long index = startIndex;

    while (1) {

        long leftIndex = LEFT_CHILD(index);
        long rightIndex = RIGHT_CHILD(index);
        long maxIndex = -1;

        if ((leftIndex <= size) &&
            (compare((void*)TM_SHARED_READ_P(elements[leftIndex]),
                     (void*)TM_SHARED_READ_P(elements[index])) > 0))
        {
            maxIndex = leftIndex;
        } else {
            maxIndex = index;
        }

        if ((rightIndex <= size) &&
            (compare((void*)TM_SHARED_READ_P(elements[rightIndex]),
                     (void*)TM_SHARED_READ_P(elements[maxIndex])) > 0))
        {
            maxIndex = rightIndex;
        }

        if (maxIndex == index) {
            break;
        } else {
            void* tmpPtr = (void*)TM_SHARED_READ_P(elements[index]);
            TM_SHARED_WRITE_P(elements[index],
                              (void*)TM_SHARED_READ(elements[maxIndex]));
            TM_SHARED_WRITE_P(elements[maxIndex], tmpPtr);
            index = maxIndex;
        }
    }
}



/* =============================================================================
 * heap_remove
 * -- Returns NULL if empty
 * =============================================================================
 */
void*
heap_remove (heap_t* heapPtr)
{
    long size = heapPtr->size.val;

    if (size < 1) {
        return NULL;
    }

    tm_obj<void*>* elements = heapPtr->elements.val;
    void* dataPtr = elements[1].val;
    elements[1].val = elements[size].val;
    heapPtr->size.val = size - 1;
    heapify(heapPtr, 1);

    return dataPtr;
}


/* =============================================================================
 * TMheap_remove
 * -- Returns NULL if empty
 * =============================================================================
 */
void*
TMheap_remove (TM_ARGDECL  heap_t* heapPtr)
{
    long size = (long)TM_SHARED_READ(heapPtr->size);

    if (size < 1) {
        return NULL;
    }

    tm_obj<void*>* elements = (tm_obj<void*>*)TM_SHARED_READ_P(heapPtr->elements);
    void* dataPtr = (void*)TM_SHARED_READ_P(elements[1]);
    TM_SHARED_WRITE_P(elements[1], TM_SHARED_READ_P(elements[size]));
    TM_SHARED_WRITE(heapPtr->size, (size - 1));
    TMheapify(TM_ARG  heapPtr, 1);

    return dataPtr;
}


/* =============================================================================
 * heap_isValid
 * =============================================================================
 */
bool_t
heap_isValid (heap_t* heapPtr)
{
    long size = heapPtr->size.val;
    long (*compare)(const void*, const void*) = heapPtr->compare;
    tm_obj<void*>* elements = heapPtr->elements.val;

    long i;
    for (i = 1; i < size; i++) {
        if (compare(elements[i+1].val, elements[PARENT(i+1)].val) > 0) {
            return FALSE;
        }
    }

    return TRUE;
}


/* =============================================================================
 * TEST_HEAP
 * =============================================================================
 */
#ifdef TEST_HEAP


#include <assert.h>
#include <stdio.h>


static long
compare (const void* a, const void* b)
{
    return (*((const long*)a) - *((const long*)b));
}


static void
printHeap (heap_t* heapPtr)
{
    printf("[");

    long i;
    for (i = 0; i < heapPtr->size; i++) {
        printf("%li ", *(long*)heapPtr->elements[i+1]);
    }

    puts("]");
}


static void
insertInt (heap_t* heapPtr, long* data)
{
    printf("Inserting: %li\n", *data);
    assert(heap_insert(heapPtr, (void*)data));
    printHeap(heapPtr);
    assert(heap_isValid(heapPtr));
}


static void
removeInt (heap_t* heapPtr)
{
    long* data = heap_remove(heapPtr);
    printf("Removing: %li\n", *data);
    printHeap(heapPtr);
    assert(heap_isValid(heapPtr));
}



long global_data[] = {3, 1, 4, 1, 5, 9, 2, 6, 5, 3, 5, 8, 9};
long global_numData = sizeof(global_data) / sizeof(global_data[0]);

int
main ()
{
    puts("Starting...");

    heap_t* heapPtr = heap_alloc(1, compare);

    assert(heapPtr);

    long i;
    for (i = 0; i < global_numData; i++) {
        insertInt(heapPtr, &global_data[i]);
    }

    for (i = 0; i < global_numData; i++) {
        removeInt(heapPtr);
    }

    assert(heap_remove(heapPtr) == NULL); /* empty */

    heap_free(heapPtr);

    puts("Passed all tests.");

    return 0;
}


#endif /* TEST_HEAP */


/* =============================================================================
 *
 * End of heap.c
 *
 * =============================================================================
 */
