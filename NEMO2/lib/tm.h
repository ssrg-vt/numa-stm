#ifndef TM_H
#define TM_H 1

#ifdef HAVE_CONFIG_H
# include "STAMP_config.h"
#endif

#  include <stdio.h>

#  define MAIN(argc, argv)              int main (int argc, char** argv)
#  define MAIN_RETURN(val)              return val

#  define GOTO_SIM()                    /* nothing */
#  define GOTO_REAL()                   /* nothing */
#  define IS_IN_SIM()                   (0)

#  define SIM_GET_NUM_CPU(var)          /* nothing */

#  define TM_PRINTF                     printf
#  define TM_PRINT0                     printf
#  define TM_PRINT1                     printf
#  define TM_PRINT2                     printf
#  define TM_PRINT3                     printf

#  define P_MEMORY_STARTUP(numThread)   /*tm_memory_init(numThread)*/
#  define P_MEMORY_SHUTDOWN()           /*tm_memory_free()*/

#  include <assert.h>
#  include "memory.h"
#  include "thread.h"
#  include "types.h"
#  include "thread.h"

//#include <immintrin.h>
//#include <rtmintrin.h>
#include "../tm/tm_thread.hpp"

#define PART_HTM

#ifdef PART_HTM

#  define TM_ARG                        tx,
#  define TM_ARG_ALONE                  tx
#  define TM_ARGDECL                    Tx_Context* tx,
#  define TM_ARGDECL_ALONE              Tx_Context* tx
#  define TM_CALLABLE                   /* nothing */

#  define TM_STARTUP(numThread)         tm_sys_init()
#  define TM_SHUTDOWN()                 /* nothing */

#  define TM_THREAD_ENTER(a, b, c)             thread_init(a, b, c)
#  define TM_THREAD_EXIT()						thread_end()

#  define TM_BEGIN_WAIVER()
#  define TM_END_WAIVER()

#  define P_MALLOC(size)                malloc(size)  
#  define SEQ_MALLOC(size)              malloc(size)
#  define P_FREE(ptr)                   /* nothing free(ptr)*/
#  define SEQ_FREE(ptr)					free(ptr)
//#  define TM_MALLOC(size)               tm_malloc(size)
#  define TM_MALLOC(size)               malloc(size)
#  define TM_FREE(ptr)                  /* nothing free(ptr)*/

//# define IS_LOCKED(lock)        *((volatile int*)(&lock)) != 0

# define AL_LOCK(idx)                    /* nothing */
# define TM_BEGIN(b) 	TM_BEGIN2

//TM_NO_SHORT_BEGIN \
//						TM_LONG_BEGIN


//{ \
//        int tries = 5;  \
//        while (1) { \
//            while (IS_LOCKED(the_lock)) { __asm__ ( "pause;" ); } \
//            int status = _xbegin(); \
//            if (status == _XBEGIN_STARTED) { break; } \
//            tries--;	\
//            if (tries <= 0) {   \
//                pthread_mutex_lock(&the_lock); \
//                break;  \
//            }   \
//        }


# define TM_END()	TM_END2

//TM_LONG_END


//if (tries > 0) {    \
//                                if (IS_LOCKED(the_lock)) _xabort(30); \
//                                _xend();    \
//                            } else {    \
//                                pthread_mutex_unlock(&the_lock);    \
//                            }\
//                        };



#    define TM_BEGIN_RO()                 TM_BEGIN
#    define TM_RESTART()                  tm_abort(tx, 1)
#    define TM_EARLY_RELEASE(var)         

#  define TM_SHARED_READ(var)         TM_READ(var)
#  define TM_SHARED_WRITE(var, val)   TM_WRITE(var, val)
#  define TM_LOCAL_WRITE(var, val)    ({var = val; var;})


#  define TM_SHARED_READ(var)         TM_READ(var)
#  define TM_SHARED_WRITE(var, val)   TM_WRITE(var, val)

#  define TM_SHARED_READ_I(var)         TM_READ(var)
#  define TM_SHARED_READ_L(var)         TM_READ(var)
#  define TM_SHARED_READ_P(var)         TM_READ(var)
#  define TM_SHARED_READ_F(var)         TM_READ(var)

#  define TM_SHARED_WRITE_I(var, val)   TM_WRITE(var, val)
#  define TM_SHARED_WRITE_L(var, val)   TM_WRITE(var, val)
#  define TM_SHARED_WRITE_P(var, val)   TM_WRITE(var, val)
#  define TM_SHARED_WRITE_F(var, val)   TM_WRITE(var, val)

#  define TM_LOCAL_WRITE_I(var, val)    ({var = val; var;})
#  define TM_LOCAL_WRITE_L(var, val)    ({var = val; var;})
#  define TM_LOCAL_WRITE_P(var, val)    ({var = val; var;})
#  define TM_LOCAL_WRITE_F(var, val)    ({var = val; var;})

#endif

#ifdef GL_PART


#  define TM_ARG                        /* nothing */
#  define TM_ARG_ALONE                  /* nothing */
#  define TM_ARGDECL                    /* nothing */
#  define TM_ARGDECL_ALONE              /* nothing */
#  define TM_CALLABLE                   /* nothing */

#  define TM_STARTUP(numThread)         /* nothing */
#  define TM_SHUTDOWN()                 /* nothing */

#  define TM_THREAD_ENTER()             thread_init()
#  define TM_THREAD_EXIT()

#  define TM_BEGIN_WAIVER()
#  define TM_END_WAIVER()

#  define P_MALLOC(size)                malloc(size)
#  define SEQ_MALLOC(size)              malloc(size)
#  define P_FREE(ptr)                   /* nothing free(ptr)*/
#  define SEQ_FREE(ptr)					free(ptr)
//#  define TM_MALLOC(size)               tm_malloc(size)
#  define TM_MALLOC(size)               malloc(size)
#  define TM_FREE(ptr)                  /* nothing free(ptr)*/

//# define IS_LOCKED(lock)        *((volatile int*)(&lock)) != 0

# define AL_LOCK(idx)                    /* nothing */
# define TM_BEGIN(b) 	TM_NO_SHORT_BEGIN \
						TM_LONG_BEGIN(b)


//{ \
//        int tries = 5;  \
//        while (1) { \
//            while (IS_LOCKED(the_lock)) { __asm__ ( "pause;" ); } \
//            int status = _xbegin(); \
//            if (status == _XBEGIN_STARTED) { break; } \
//            tries--;	\
//            if (tries <= 0) {   \
//                pthread_mutex_lock(&the_lock); \
//                break;  \
//            }   \
//        }


#    define TM_END()	TM_LONG_END


//if (tries > 0) {    \
//                                if (IS_LOCKED(the_lock)) _xabort(30); \
//                                _xend();    \
//                            } else {    \
//                                pthread_mutex_unlock(&the_lock);    \
//                            }\
//                        };



#    define TM_BEGIN_RO()                 TM_BEGIN(0)
#    define TM_RESTART()                  /* nothing */
#    define TM_EARLY_RELEASE(var)

#  define TM_SHARED_READ(var)         (var)
#  define TM_SHARED_WRITE(var, val)   ({var = val; var;})
#  define TM_LOCAL_WRITE(var, val)    ({var = val; var;})


#  define TM_SHARED_READ(var)         (var)
#  define TM_SHARED_WRITE(var, val)   ({var = val; var;})

#  define TM_SHARED_READ_I(var)         (var)
#  define TM_SHARED_READ_L(var)         (var)
#  define TM_SHARED_READ_P(var)         (var)
#  define TM_SHARED_READ_F(var)         (var)

#  define TM_SHARED_WRITE_I(var, val)   ({var = val; var;})
#  define TM_SHARED_WRITE_L(var, val)   ({var = val; var;})
#  define TM_SHARED_WRITE_P(var, val)   ({var = val; var;})
#  define TM_SHARED_WRITE_F(var, val)   ({var = val; var;})

#  define TM_LOCAL_WRITE_I(var, val)    ({var = val; var;})
#  define TM_LOCAL_WRITE_L(var, val)    ({var = val; var;})
#  define TM_LOCAL_WRITE_P(var, val)    ({var = val; var;})
#  define TM_LOCAL_WRITE_F(var, val)    ({var = val; var;})

#endif


#endif
