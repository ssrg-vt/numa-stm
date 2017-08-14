#define OPTIK_DELETED ((uint64_t) -1)
#define OPTIK_INIT    0
#define OPTIK_LOCKED  0x1LL //odd values -> locked

typedef volatile uint64_t optik_t;

inline int optik_is_locked(optik_t ol)
{
    return (ol & OPTIK_LOCKED);
}

inline optik_t optik_get_version_wait(optik_t* ol)
{
    do {
        optik_t olv = *ol;
        if ((!optik_is_locked(olv))) {
            return olv;
        }
        cpause(128);
    }
    while (1);
}

inline int optik_is_deleted(optik_t ol)
{
    return (ol == OPTIK_DELETED);
}

inline uint32_t optik_get_version(optik_t ol)
{
    return ol;
}

inline uint32_t optik_get_n_locked(optik_t ol)
{
    return ol >> 1;
}

inline void optik_init(optik_t* ol)
{
    *ol = 0;
}

inline int optik_is_same_version(optik_t v1, optik_t v2)
{
    return v1 == v2;
}

inline int optik_trylock_version(optik_t* ol, optik_t ol_old)
{
    if ((optik_is_locked(ol_old) || *ol != ol_old)) {
        return 0;
    }

    return __sync_val_compare_and_swap(ol, ol_old, ol_old + 1) == ol_old;
}

inline int optik_trylock_vdelete(optik_t* ol, optik_t ol_old)
{
    if ((optik_is_locked(ol_old) || *ol != ol_old)) {
        return 0;
    }

    return __sync_val_compare_and_swap(ol, ol_old, OPTIK_DELETED) == ol_old;
}

inline int optik_lock(optik_t* ol)
{
    optik_t ol_old;
    do {
        while (1) {
            ol_old = *ol;
            if (!optik_is_locked(ol_old)) {
                break;
            }
            asm volatile("mfence");
        }
        if (__sync_val_compare_and_swap(ol, ol_old, ol_old + 1) == ol_old) {
            break;
        }
    } while (1);
    return 1;
}

inline int optik_lock_backoff(optik_t* ol)
{
    optik_t ol_old;
    do {
        while (1) {
            ol_old = *ol;
            if (!optik_is_locked(ol_old)) {
                break;
            }
            cpause(128);
        }

        if (__sync_val_compare_and_swap(ol, ol_old, ol_old + 1) == ol_old) {
            break;
        }
    } while (1);
    return 1;
}

inline int optik_lock_version(optik_t* ol, optik_t ol_old)
{
    optik_t ol_cur;
    do {
        while (1) {
            ol_cur = *ol;
            if (!optik_is_locked(ol_cur)) {
                break;
            }
            asm volatile("mfence");
        }

        if (__sync_val_compare_and_swap(ol, ol_cur, ol_cur + 1) == ol_cur) {
            break;
        }
    } while (1);
    return ol_cur == ol_old;
}

inline int optik_lock_version_backoff(optik_t* ol, optik_t ol_old)
{
    optik_t ol_cur;
    do {
        while (1) {
            ol_cur = *ol;
            if (!optik_is_locked(ol_cur)) {
                break;
            }
            cpause(128);
        }

        if (__sync_val_compare_and_swap(ol, ol_cur, ol_cur + 1) == ol_cur) {
            break;
        }
    } while (1);
    return ol_cur == ol_old;
}

inline int optik_trylock(optik_t* ol)
{
    optik_t ol_new = *ol;
    if ((optik_is_locked(ol_new))) {
        return 0;
    }
    return __sync_val_compare_and_swap(ol, ol_new, ol_new + 1) == ol_new;
}

inline void optik_unlock(optik_t* ol)
{
    asm volatile ("sfence");
    __sync_fetch_and_add(ol, 1);
}

inline optik_t optik_unlockv(optik_t* ol)
{
    return (optik_t) __sync_add_and_fetch(ol, 1);
}

inline void optik_revert(optik_t* ol)
{
    __sync_fetch_and_sub(ol, 1);
}
