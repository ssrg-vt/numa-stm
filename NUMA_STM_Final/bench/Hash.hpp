/**
 *  Copyright (C) 2011
 *  University of Rochester Department of Computer Science
 *    and
 *  Lehigh University Department of Computer Science and Engineering
 *
 * License: Modified BSD
 *          Please see the file LICENSE.RSTM for licensing information
 */

#ifndef HASH_HPP__
#define HASH_HPP__

#include "List.hpp"

// the Hash class is an array of N_BUCKETS LinkedLists
class HashTable
{
    static const int N_BUCKETS = 256;
    static const int A_BUCKETS = 256 * 8;
    /**
     *  during a sanity check, we want to make sure that every element in a
     *  bucket actually hashes to that bucket; we do it by passing this
     *  method to the extendedSanityCheck for the bucket.
     */
    static bool verify_hash_function(uint32_t val, uint32_t zone, uint32_t bucket)
    {
    	int hash = val % A_BUCKETS;
        return ((hash/(N_BUCKETS)) == zone) && ((hash%(N_BUCKETS)) == bucket);
    }

  public:
    /**
     *  Templated type defines what kind of list we'll use at each bucket.
     */
    List* bucket[8][N_BUCKETS];

    HashTable() {
    	for (int i=0; i<8; i++) {
    		for (int j=0; j<N_BUCKETS; j++) {
    			bucket[i][j] = new List(i);
    		}
    	}
    }


    void insert(int val TM_ARG)
    {
    	int hash = val % A_BUCKETS;
        bucket[hash/(N_BUCKETS)][hash%(N_BUCKETS)]->insert(val TM_PARAM);
    }


    void insert_NoTM(int val)
    {
    	int hash = val % A_BUCKETS;
        bucket[hash/(N_BUCKETS)][hash%(N_BUCKETS)]->insert_NoTM(val);
    }

    bool lookup(int val TM_ARG) const
    {
    	int hash = val % A_BUCKETS;
        return bucket[hash/(N_BUCKETS)][hash%(N_BUCKETS)]->lookup(val TM_PARAM);
    }

    void remove(int val TM_ARG)
    {
    	int hash = val % A_BUCKETS;
    	bucket[hash/(N_BUCKETS)][hash%(N_BUCKETS)]->remove(val TM_PARAM);
    }

    bool isSane() const
    {
    	for (int i=0; i<8 ;i++)
        for (int j = 0; j < N_BUCKETS; j++)
            if (!bucket[i][j]->extendedSanityCheck(verify_hash_function, i, j))
                return false;
        return true;
    }
};

#endif // HASH_HPP__
