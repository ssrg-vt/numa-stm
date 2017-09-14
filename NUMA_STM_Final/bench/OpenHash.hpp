/**
 *  Copyright (C) 2011
 *  University of Rochester Department of Computer Science
 *    and
 *  Lehigh University Department of Computer Science and Engineering
 *
 * License: Modified BSD
 *          Please see the file LICENSE.RSTM for licensing information
 */

#ifndef OPENHASH_HPP__
#define OPENHASH_HPP__

// the Hash class is an array of N_BUCKETS LinkedLists
class OpenHash
{
      /***  data type for the index */
      struct index_t
      {
          size_t version;
          void*  address;
          size_t index;

          index_t() : version(0), address(NULL), index(0) { }
      };

      index_t* index;                             // hash entries
      size_t   shift;                             // for the hash function
      size_t   ilength;                           // max size of hash
      size_t   version;                           // version for fast clearing

      WriteSetEntry* list;                        // the array of actual data
      size_t   capacity;                          // max array size
      size_t   lsize;                             // elements in the array


      /**
       *  hash function is straight from CLRS (that's where the magic
       *  constant comes from).
       */
      size_t hash(void* const key) const
      {
          static const unsigned long long s = 2654435769ull;
          const unsigned long long r = ((unsigned long long)key) * s;
          return (size_t)((r & 0xFFFFFFFF) >> shift);
      }

      /**
       *  This doubles the size of the index. This *does not* do anything as
       *  far as actually doing memory allocation. Callers should delete[]
       *  the index table, increment the table size, and then reallocate it.
       */
      size_t doubleIndexLength();

      /**
       *  Supporting functions for resizing.  Note that these are never
       *  inlined.
       */
      void rebuild();
      void resize();
      void reset_internal();

    public:

      WriteSet(const size_t initial_capacity);
      ~WriteSet();

      /**
       *  Search function.  The log is an in/out parameter, and the bool
       *  tells if the search succeeded. When we are byte-logging, the log's
       *  mask is updated to reflect the bytes in the returned value that are
       *  valid. In the case that we don't find anything, the mask is set to 0.
       */
      bool find(WriteSetEntry& log) const
      {
          size_t h = hash(log.addr);

          while (index[h].version == version) {
              if (index[h].address != log.addr) {
                  // continue probing
                  h = (h + 1) % ilength;
                  continue;
              }
              log.val = list[index[h].index].val;
              return true;
          }

          return false;
      }

      /**
       *  Support for abort-on-throw and stack protection makes rollback
       *  tricky.  We might need to write to an exception object, and/or
       *  filter writeback to protect the stack.
       *
       *  NB: We use a macro to hide the fact that some rollback calls are
       *      really simple.  This gets called by ~30 STM implementations
       */
#if !defined (STM_ABORT_ON_THROW)
      void rollback() { }
#   define STM_ROLLBACK(log, stack, exception, len) log.rollback()
#else
#   if !defined(STM_PROTECT_STACK)
      void rollback(void**, size_t);
#   define STM_ROLLBACK(log, stack, exception, len) log.rollback(exception, len)
#   else
      void rollback(void**, void**, size_t);
#   define STM_ROLLBACK(log, stack, exception, len) log.rollback(stack, exception, len)
#   endif
#endif

      /**
       *  Encapsulate writeback in this routine, so that we can avoid making
       *  modifications to lots of STMs when we need to change writeback for a
       *  particular compiler.
       */
#if !defined(STM_PROTECT_STACK)
      __attribute__((always_inline)) void writeback()
      {
#else
      TM_INLINE void writeback(void** upper_stack_bound)
      {
#endif
          for (iterator i = begin(), e = end(); i != e; ++i)
          {
#ifdef STM_PROTECT_STACK
              // See if this falls into the protected stack region, and avoid
              // the writeback if that is the case. The filter call will update
              // a byte-log's mask if there is an awkward intersection.
              //
              void* top_of_stack;
              if (i->filter(&top_of_stack, upper_stack_bound))
                  continue;
#endif
              i->writeback();
          }
      }

      __attribute__((always_inline)) bool validate()
		{
			for (iterator i = begin(), e = end(); i != e; ++i)
			{
				if (!i->validate()) return false;
			}
			return true;
		}
      /**
       *  Inserts an entry in the write set.  Coalesces writes, which can
       *  appear as write reordering in a data-racy program.
       */
      bool insert(const WriteSetEntry& log)
      {
          size_t h = hash(log.addr);

          //  Find the slot that this address should hash to. If we find it,
          //  update the value. If we find an unused slot then it's a new
          //  insertion.
          while (index[h].version == version) {
              if (index[h].address != log.addr) {
                  h = (h + 1) % ilength;
                  continue; // continue probing at new h
              }

              // there /is/ an existing entry for this word, we'll be updating
              // it no matter what at this point
              list[index[h].index].update(log);
              return true;
          }

          // add the log to the list (guaranteed to have space)
          list[lsize] = log;

          // update the index
          index[h].address = log.addr;
          index[h].version = version;
          index[h].index   = lsize;

          // update the end of the list
          lsize += 1;

          // resize the list if needed
          if (__builtin_expect(lsize == capacity, false))
              resize();

          // if we reach our load-factor
          // NB: load factor could be better handled rather than the magic
          //     constant 3 (used in constructor too).
          if (__builtin_expect((lsize * 3) >= ilength, false))
              rebuild();

          return false;
      }

      /*** size() lets us know if the transaction is read-only */
      size_t size() const { return lsize; }

      /**
       *  We use the version number to reset in O(1) time in the common case
       */
      void reset()
      {
          lsize    = 0;
          version += 1;

          // check overflow
          if (version != 0)
              return;
          reset_internal();
      }

      /*** Iterator interface: iterate over the list, not the index */
      typedef WriteSetEntry* iterator;
      iterator begin() const { return list; }
      iterator end()   const { return list + lsize; }





































	struct table_entry {
		int val;
		bool isUsed = false;
	};

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

    OpenHash() {
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
