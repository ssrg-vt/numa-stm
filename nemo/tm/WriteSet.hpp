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
 *  The RSTM backends that use redo logs all rely on this datastructure,
 *  which provides O(1) clear, insert, and lookup by maintaining a hashed
 *  index into a vector.
 */

#ifndef WRITESET_HPP__
#define WRITESET_HPP__


#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

template <typename T>
struct tm_obj {
	volatile uint64_t ver;
	volatile int lock;
	volatile int* volatile lock_p;
	T val;
};

namespace
{
  /**
   * We use malloc a couple of times here, and this makes it a bit easier
   */
  template <typename T>
  inline T* typed_malloc(size_t N__)
  {
      return static_cast<T*>(malloc(sizeof(T) * N__));
  }
}

namespace stm
{
  /**
   *  The WriteSet implementation is heavily influenced by the configuration
   *  parameters, STM_WS_(WORD/BYTE)LOG, STM_PROTECT_STACK, and
   *  STM_ABORT_ON_THROW. This means that much of this file is ifdeffed
   *  accordingly.
   */

  /**
   * The log entry type when we're word-logging is pretty trivial, and just
   * logs address/value pairs.
   */
  struct WordLoggingWriteSetEntry
  {
      void** addr;
      union {
		  uint8_t i8;
		  uint16_t i16;
		  uint32_t i32;
		  uint64_t i64;
	  } val;
      uint8_t size;

      WordLoggingWriteSetEntry(void** paddr)
          : addr(paddr), size(0)
      { val.i64 = 0;}

      WordLoggingWriteSetEntry(void** paddr, uint64_t pval, uint8_t psize)
          : addr(paddr), size(psize)
      { val.i64 = pval; }

      /**
       *  Called when we are WAW an address, and we want to coalesce the
       *  write. Trivial for the word-based writeset, but complicated for the
       *  byte-based version.
       */
      void update(const WordLoggingWriteSetEntry& rhs) { val.i64 = rhs.val.i64; }

      /**
       * Check to see if the entry is completely contained within the given
       * address range. We have some preconditions here w.r.t. alignment and
       * size of the range. It has to be at least word aligned and word
       * sized. This is currently only used with stack addresses, so we don't
       * include asserts because we don't want to pay for them in the common
       * case writeback loop.
       */
//      bool filter(void** lower, void** upper)
//      {
//          return !(addr + 1 < lower || addr >= upper);
//      }

      /**
       * Called during writeback to actually perform the logged write. This is
       * trivial for the word-based set, but the byte-based set is more
       * complicated.
       */
      void writeback() const {
//    	  ((tm_obj*)addr)->val = val.i64;
    	  switch(size) {
			  case 1:
//				*((uint8_t*)addr) = val.i8;
				(((tm_obj<uint8_t>*)addr)->val) = val.i8;
			  break;
			  case 2:
//				*((uint16_t*)addr) = val.i16;
				(((tm_obj<uint16_t>*)addr)->val) = val.i16;
			  break;
			  case 4:
//				*((uint32_t*)addr) = val.i32;
				  (((tm_obj<uint32_t>*)addr)->val) = val.i32;
			  break;
			  case 8:
//				*((uint64_t*)addr) = val.i64;
				  (((tm_obj<uint64_t>*)addr)->val) = val.i64;
			  break;
		  }
      }

      bool validate() const {
		  switch(size) {
			  case 1:
				return *((uint8_t*)addr) == val.i8;
			  break;
			  case 2:
				return *((uint16_t*)addr) == val.i16;
			  break;
			  case 4:
				return *((uint32_t*)addr) == val.i32;
			  break;
			  case 8:
				return *((uint64_t*)addr) == val.i64;
			  break;
		  }
		  //shouldn't be reached
		  return false;
	  }
      /**
       * Called during rollback if there is an exception object that we need to
       * perform writes to. The address range is the range of addresses that
       * we're looking for. If this log entry is contained in the range, we
       * perform the writeback.
       *
       * NB: We're assuming a pretty well defined address range, in terms of
       *     size and alignment here, because the word-based writeset can only
       *     handle word-sized data.
       */
//      void rollback(void** lower, void** upper)
//      {
//          if (addr >= lower && (addr + 1) <= upper)
//              writeback();
//      }
  };

//  struct obj {
//	  uint64_t id;
//	  uint64_t ver;
//	  uint64_t val;
//  };
//
//  struct BankLoggingWriteSetEntry
//    {
//	  	obj* addr;
//        obj  val;
//
//        BankLoggingWriteSetEntry(obj* paddr, obj pval)
//            : addr(paddr), val(pval)
//        { }
//
//        BankLoggingWriteSetEntry(obj* paddr)
//		    : addr(paddr)
//	    { }
//        /**
//         *  Called when we are WAW an address, and we want to coalesce the
//         *  write. Trivial for the word-based writeset, but complicated for the
//         *  byte-based version.
//         */
//        void update(const BankLoggingWriteSetEntry& rhs) { val = rhs.val; }
//
//        /**
//         * Check to see if the entry is completely contained within the given
//         * address range. We have some preconditions here w.r.t. alignment and
//         * size of the range. It has to be at least word aligned and word
//         * sized. This is currently only used with stack addresses, so we don't
//         * include asserts because we don't want to pay for them in the common
//         * case writeback loop.
//         */
////        bool filter(void** lower, void** upper)
////        {
////            return !(addr + 1 < lower || addr >= upper);
////        }
//
//        /**
//         * Called during writeback to actually perform the logged write. This is
//         * trivial for the word-based set, but the byte-based set is more
//         * complicated.
//         */
//        void writeback()  {val.ver++; *addr = val; }
//
//        /**
//         * Called during rollback if there is an exception object that we need to
//         * perform writes to. The address range is the range of addresses that
//         * we're looking for. If this log entry is contained in the range, we
//         * perform the writeback.
//         *
//         * NB: We're assuming a pretty well defined address range, in terms of
//         *     size and alignment here, because the word-based writeset can only
//         *     handle word-sized data.
//         */
////        void rollback(void** lower, void** upper)
////        {
////            if (addr >= lower && (addr + 1) <= upper)
////                writeback();
////        }
//    };
  /**
   *  Pick a write-set implementation, based on the configuration.
   */
//typedef BankLoggingWriteSetEntry WriteSetEntry;

typedef WordLoggingWriteSetEntry WriteSetEntry;
#define STM_WRITE_SET_ENTRY(addr, val, mask) addr, val

  /**
   *  The write set is an indexed array of WriteSetEntry elements.  As with
   *  MiniVector, we make sure that certain expensive but rare functions are
   *  never inlined.
   */
  class WriteSet
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

      void remove(const WriteSetEntry& log) {
    	  size_t h = hash(log.addr);

    	  while (index[h].version == version) {
 			if (index[h].address != log.addr) {
				h = (h + 1) % ilength;
				continue; // continue probing at new h
			}

			// there /is/ an existing entry for this word, we'll be updating
			// it no matter what at this point
			index[h].version--;
		  }
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
  };


}

#endif // WRITESET_HPP__
