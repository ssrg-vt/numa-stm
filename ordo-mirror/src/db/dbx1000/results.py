#!/usr/bin/env python3

import os
import sys

algs = ['DL_DETECT', 'NO_WAIT', 'HEKATON', 'SILO', 'TICTOC', 'OCC', 'MVCC']
outdir = "output"


def get_stats(s, c):
    s = s.split('\n')
    s = s[-2].split('=')
    txns = int(s[1].split(', ')[0])
    abrts = int(s[2].split(', ')[0])
    secs = float(s[3].split(', ')[0])
    return (float(txns * c)/secs, float(abrts * c) / secs)


def get_ycsb_stats_with_contention(cores, sockets, algo, ctype):
    print("# %s: %s contention" % (algo, ctype))
    print("# Cores\tTxns/sec\tAborts")
    cores_per_socket = cores / (2 * sockets)
    for c in [1] + [x*cores_per_socket for x in range(1, sockets * 2 + 1)]:
        output = ''
        with open(os.path.join(outdir, "ycsb-ctn-%s-algo-%s-c%d" %
            (ctype, algo, c)), 'r') as f:
            output = f.read()
        (txns_per_sec, abrts) = get_stats(output, c)
        print("%d\t%f\t%f" % (c, txns_per_sec, abrts))
    print("\n\n")


def get_ycsb_stats(cores, sockets, algo):
    for i in algo:
        # no contention
        get_ycsb_stats_with_contention(cores, sockets, i, "no")
        # medium contention
        get_ycsb_stats_with_contention(cores, sockets, i, "med")
        # high contention
        get_ycsb_stats_with_contention(cores, sockets, i, "high")


def main():
    if (len(sys.argv) < 2):
        print("%s max-cores num-sockets algo" % (sys.argv[0]))
        exit(-1)

    num_sockets = 8
    algo = algs

    if len(sys.argv) >= 3:
        num_sockets = int(sys.argv[2])
    if len(sys.argv) == 4:
        algo = []
        if sys.argv[3] in algs:
            algo.append(sys.argv[3])
        else:
            print("%s max-cores num-sockets algo" % (sys.argv[0]))
            exit(-1)

    cores = int(sys.argv[1])
    get_ycsb_stats(cores, num_sockets, algo)
    # run_tpcc(cores, num_sockets, algo)

if __name__ == '__main__':
    main()
