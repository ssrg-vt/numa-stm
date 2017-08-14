#ifndef __LIB_TIMESTAMP_H_

#ifndef __KERNEL__
#include <stdint.h>
#else
#define uint64_t u64
#endif

/*
 * @get_unordered_core_timestamp:
 * returns rdtscp() result without orderign
 */
inline uint64_t get_unordered_core_timestamp(void);

/*
 * @get_ordered_core_timestamp:
 * returns rdtscp() which is ordered
 */
inline uint64_t get_ordered_core_timestamp(void);

/*
 * @get_global_epoch_timestamp:
 * provides the epoch that keeps on increasing on its own
 */
inline uint64_t get_global_epoch_timestamp(void);

#endif /* __LIB_TIMESTAMP_H_ */
