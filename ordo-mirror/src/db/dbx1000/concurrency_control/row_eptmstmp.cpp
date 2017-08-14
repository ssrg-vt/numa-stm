#include "row_eptmstmp.h"
#include "row.h"
#include "txn.h"
#include "mem_alloc.h"
#include <mm_malloc.h>

#if CC_ALG==EPTMSTMP

void
Row_eptmstmp::init(row_t * row)
{
	_row = row;
	_wts = 0;
	_rts = 0;
    optik_init(&_o);
}

RC
Row_eptmstmp::access(txn_man * txn, TsType type, row_t * local_row)
{
    for (;;) {
        optik_t vn = optik_get_version_wait(&_o);
        txn->last_wts = _wts;
        txn->last_rts = txn->start_ts;
        local_row->copy(_row);
        optik_t vnc = optik_get_version(_o);
        if (optik_is_same_version(vn, vnc))
            break;
    }
    return RCOK;
}

void
Row_eptmstmp::write_data(row_t * data, ts_t wts)
{
	_row->copy(data);
	_wts = wts;
	_rts = wts;
    smp_wmb();
}

bool
Row_eptmstmp::validate(ts_t wts, ts_t rts, ts_t &new_rts, optik_t &v)
{
    optik_t nv = optik_get_version_wait(&_o);
	if (wts + CLOCK_DIFF < _wts)
		return false;
    return optik_is_same_version(v, nv);
}


ts_t
Row_eptmstmp::get_wts()
{
	return (*(volatile ts_t *)&_wts);
}

void
Row_eptmstmp::get_ts_word(uint64_t &rts, uint64_t &wts, optik_t &v)
{
	wts = get_wts();
	rts = get_rts();
    v = optik_get_version(_o);
}

ts_t
Row_eptmstmp::get_rts()
{
	return (*(volatile ts_t *)&_rts);
}

void
Row_eptmstmp::lock()
{
	optik_lock(&_o);
}

bool
Row_eptmstmp::try_lock()
{
	return optik_trylock(&_o);
}

void
Row_eptmstmp::release()
{
    optik_unlock(&_o);
}

void
Row_eptmstmp::revert()
{
    optik_revert(&_o);
}

#endif
