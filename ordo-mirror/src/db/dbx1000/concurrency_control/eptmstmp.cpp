#include "txn.h"
#include "row.h"
#include "row_eptmstmp.h"
#include "manager.h"

#if CC_ALG == EPTMSTMP

/*
 * NOTE: Implementatin repeatable reads is straight forward. We have
 * already made a COHERENT copy of the read set, so we can proceed forward
 * without checking the validity of the read set.
 * Excerpt from:  http://stackoverflow.com/questions/4034976/difference-between-read-commit-and-repeatable-read
 * Read committed is an isolation level that guarantees that any data read
 * was committed at the moment is read. It simply restricts the reader from
 * seeing any intermediate, uncommitted, 'dirty' read. IT makes no promise
 * whatsoever that if the transaction re-issues the read, will find the Same
 * data, data is free to change after it was read.
 *
 * Repeatable read is a higher isolation level, that in addition to the
 * guarantees of the read committed level, it also guarantees that any data
 * read cannot change, if the transaction reads the same data again, it will
 * find the previously read data in place, unchanged, and available to read.
 *
 * The next isolation level, serializable, makes an even stronger guarantee:
 * in addition to everything repeatable read guarantees, it also guarantees
 * that no new data can be seen by a subsequent read.
 */

RC
txn_man::validate_eptmstmp()
{
    RC rc = RCOK;
    int num_locks = 0;
    int write_set[wr_cnt];
    int cur_wr_idx = 0;
    ts_t commit_wts = 0;
#if ISOLATION_LEVEL != REPEATABLE_READ
    int read_set[row_cnt - wr_cnt];
    optik_t rsv[row_cnt - wr_cnt];
    int cur_rd_idx = 0;
#endif

    /* Enumerating the accesses in read/write set */
    for (int rid = 0; rid < row_cnt; rid ++) {
        if (accesses[rid]->type == WR)
            write_set[cur_wr_idx ++] = rid;
#if ISOLATION_LEVEL != REPEATABLE_READ
        else
            read_set[cur_rd_idx ++] = rid;
#endif
    }

    // bubble sort the write_set, in primary key order
    // This is the canonical form to avoid deadlocks
    for (int i = wr_cnt - 1; i >= 1; i--) {
        for (int j = 0; j < i; j++) {
            if (accesses[ write_set[j] ]->orig_row->get_primary_key() >
                accesses[ write_set[j + 1] ]->orig_row->get_primary_key())
            {
                int tmp = write_set[j];
                write_set[j] = write_set[j+1];
                write_set[j+1] = tmp;
            }
        }
    }


    bool done = false;
    if (_pre_abort) {
        /* Check whether wts has changed, if yes then abort */
        for (int i = 0; i < wr_cnt; i++) {
            row_t * row = accesses[ write_set[i] ]->orig_row;
            if (row->manager->get_wts() != accesses[write_set[i]]->wts) {
                rc = encode_abrt_type(ABORT_WW);
                INC_STATS(get_thd_id(), debug1, 1);
                goto final;
            }
        }

#if ISOLATION_LEVEL == SERIALIZABLE
        /*
         * Check the read set: if rts is bigger than wts, then check whether
         * wts is same. This one tells about read-clobber aborts.
         */
        for (int i = 0; i < row_cnt - wr_cnt; i++) {
            row_t * row = accesses[read_set[i]]->orig_row;
            uint64_t wts, rts;
            row->manager->get_ts_word(rts, wts, rsv[i]);
            if (wts != accesses[read_set[i]]->wts) {
                rc = encode_abrt_type(ABORT_RW);
                INC_STATS(get_thd_id(), debug2, 1);
                goto final;
            }
        }
#endif
    }

    /* _validation_no_wait is always true */
    if (_validation_no_wait) {
        while (!done) {
            /* Now, try to acquire the row lock */
            num_locks = 0;
            for (int i = 0; i < wr_cnt; i++) {
                row_t * row = accesses[write_set[i]]->orig_row;
                if (!row->manager->try_lock())
                    break;
                num_locks ++;
                if (row->manager->get_wts() != accesses[write_set[i]]->wts) {
                    rc = encode_abrt_type(ABORT_WW);
                    INC_STATS(get_thd_id(), debug3, 1);
                    goto final;
                }
            }

            if (num_locks == wr_cnt)
                done = true;
            else {
                for (int i = 0; i < num_locks; i++)
                    accesses[write_set[i]]->orig_row->manager->revert();
                if (_pre_abort) {
                    num_locks = 0;
                    for (int i = num_locks; i < wr_cnt; i++) {
                        row_t * row = accesses[write_set[i]]->orig_row;
                        if (row->manager->get_wts() >
                            accesses[write_set[i]]->wts + CLOCK_DIFF) {
                            rc = encode_abrt_type(ABORT_WW);
                            INC_STATS(get_thd_id(), debug4, 1);
                            goto final;
                        }
                    }
#if ISOLATION_LEVEL == SERIALIZABLE
                    for (int i = 0; i < row_cnt - wr_cnt; i++) {
                        Access * access = accesses[read_set[i]];
                        uint64_t wts, rts;
                        optik_t v;
                        access->orig_row->manager->get_ts_word(rts, wts, v);
                        if (wts > access->wts + CLOCK_DIFF) {
                                rc = encode_abrt_type(ABORT_RW);
                                INC_STATS(get_thd_id(), debug5, 1);
                                goto final;
                        }
                    }
#endif
                }
                usleep(1);
            }
        }
    }
    else { // _validation_no_wait = false
        for (int i = 0; i < wr_cnt; i++) {
            row_t * row = accesses[write_set[i]]->orig_row;
            row->manager->lock();
            num_locks++;
            if (row->manager->get_wts() != accesses[write_set[i]]->wts) {
                rc = encode_abrt_type(ABORT_WW);
                INC_STATS(get_thd_id(), debug6, 1);
                goto final;
            }
        }
    }

    commit_wts = get_sys_clock();

    assert (num_locks == wr_cnt);
    // Validate the read set.
#if ISOLATION_LEVEL == SERIALIZABLE
    for (int i = 0; i < row_cnt - wr_cnt; i ++) {
	    Access * access = accesses[read_set[i]];
	    if (access->rts > commit_wts - CLOCK_DIFF) {
		    bool success =
			    access->orig_row->manager->validate(access->wts,
								 commit_wts, access->rts, rsv[i]);
		    if (!success) {
                rc = encode_abrt_type(ABORT_RW);
			    INC_STATS(get_thd_id(), debug7, 1);
			    rc = Abort;
			    goto final;
		    }
	    }
    }
#endif
 final:
    if (get_rc(rc) == Abort) {
	    for (int i = 0; i < num_locks; i++)
		    accesses[write_set[i]]->orig_row->manager->revert();
	    cleanup(rc);
    } else {
        for (int i = 0; i < wr_cnt; i++) {
            Access * access = accesses[write_set[i]];
            access->orig_row->manager->write_data(access->data, commit_wts);
            access->orig_row->manager->release();
        }
        if (g_prt_lat_distr)
            stats.add_debug(get_thd_id(), commit_wts, 2);
        cleanup(rc);
    }
    return rc;
}
#endif
