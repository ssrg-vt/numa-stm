#ifndef __UTIL_H
#define __UTIL_H

#define CAS(addr, oldv, newv) __sync_bool_compare_and_swap((addr), (oldv), (newv))

#define MEMBARSTLD() __sync_synchronize()

#define FETCH_AND_ADD(addr, v) __sync_fetch_and_add((addr), (v))

static inline unsigned long rdtscp(void)
{
    unsigned long a, d;
    __asm__ volatile("rdtscp" : "=a"(a), "=d"(d));
    /* __asm __volatile("rdtscp; mov %%eax, %0; mov %%edx, %1; cpuid"
                     : "=r" (a), "=r" (d)
                     : : "%rax", "%rbx", "%rcx", "%rdx");
    */
    return a | (d << 32UL);
}

#endif
