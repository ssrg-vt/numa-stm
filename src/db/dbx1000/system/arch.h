#ifndef __ARCH_H_
#define __ARCH_H_
inline uint64_t
swap_uint64(volatile uint64_t* target,  uint64_t x) {
	__asm__ __volatile__("xchgq %0,%1"
			     :"=r" ((uint64_t) x)
			     :"m" (*(volatile uint64_t *)target), "0" ((uint64_t) x)
			     :"memory");
	return x;
}

inline void smp_rmb(void)
{
    asm volatile("lfence":::"memory");
}

inline void smp_wmb(void)
{
    asm volatile("sfence":::"memory");
}

inline void smp_mb(void)
{
    asm volatile("mfence":::"memory");
}

inline void cpu_relax(void)
{
    asm volatile("rep; nop");
}

#define likely(x)   __builtin_expect((x), 1)
#define unlikely(x) __builtin_expect((x), 0)

inline void cpause(unsigned long cycles)
{
    for (unsigned long i = 0; i < cycles; ++i)
        __asm__ __volatile__("nop");
}

#endif
