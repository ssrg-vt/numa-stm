
#include <libstmp.h>

#ifdef __KERNEL__
static __always_inline uint64_t rdtscp(void)
{
	DECLARE_ARGS(val, low, high);
	asm volatile("rdtscp", EAX_EDX_RET(val, low, high));
	return EAX_EDX_VAL(val, low, high);
}

static __always_inline uint64_t rdtscp_ordered(void)
{
	alternative_2("", "mfence", X86_FEATURE_MFENCE_RDTSC,
		      "lfence", X86_FEATURE_LFENCE_RDTSC);
	return rdtscp();
}

inline uint64_t get_unordered_core_timestamp(void)
{
	return rdtscp();
}

inline uint64_t get_ordered_core_timestamp(void)
{
	return rdtscp_ordered();
}

inline uint64_t get_global_epoch_timestamp(void)
{
	return get_ordered_core_timestamp();
}
#else

/*
 * @get_unordered_core_timestamp:
 * returns rdtscp() result without orderign
 */
inline uint64_t get_unordered_core_timestamp(void)
{
	uint32_t a, d;
	__asm __volatile("rdtscp": "=a"(a), "=d"(d));
	return ((uint64_t)a) | (((uint64_t)d) << 32);
}

inline uint64_t get_ordered_core_timestamp(void)
{
	uint32_t a, d;
	__asm __volatile("rdtscp; mov %%eax, %0; mov %%edx, %1; cpuid"
			 : "=r" (a), "=r" (d)
			 : : "%rax", "%rbx", "%rcx", "%rdx");
	return ((uint64_t) a) | (((uint64_t) d) << 32);
}

inline uint64_t get_global_epoch_timestamp(void)
{
	return get_ordered_core_timestamp();
}
#endif
