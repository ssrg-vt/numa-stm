#pragma once

#include "global.h"
#include "helper.h"

#if CC_ALG == EPTMSTMP

class txn_man;
class row_t;

class Row_eptmstmp {
public:
	void 				init(row_t * row);
    RC 				    access(txn_man * txn, TsType type,
                               row_t * local_row);
	void				write_data(row_t * data, ts_t wts);
	bool 				validate(ts_t wts, ts_t rts,
                                  ts_t &new_rts, optik_t &v);

	void 				lock();
	bool  				try_lock();
	void 				release();
    void                revert();

	ts_t 				get_wts();
	ts_t 				get_rts();
	void 				get_ts_word(uint64_t &rts, uint64_t &wts,
                                    optik_t &v);
private:
	row_t * 			_row;
	ts_t 				_wts; // last write timestamp
	ts_t 				_rts; // end lease timestamp
	optik_t             _o;
};

#endif
