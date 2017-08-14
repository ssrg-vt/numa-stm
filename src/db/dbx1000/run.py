#!/usr/bin/env python3

import os
import sys
import re
import shlex
import subprocess

algs = [
    'DL_DETECT',
    'NO_WAIT',
    'SILO',
    'TICTOC',
    'HEKATON',
    'MVCC'
    'OCC',
    'EPTMSTMP',
]

outdir = "output"


def run(args):
    args = shlex.split(args)
    p = subprocess.Popen(args, stdout=subprocess.PIPE,
                         stderr=subprocess.PIPE)
    return p.communicate()


def get_stats(s, c):
    s = s.split('\n')
    s = s[-2].split('=')
    txns = int(s[1].split(', ')[0])
    abrts = int(s[2].split(', ')[0])
    secs = float(s[4].split(', ')[0])
    return (float(txns * c)/secs, float(abrts * c) / secs)


def compile_code():
    run("make clean")
    run("make -j")


def update_config_file(param, value):
    old = r"\#define\s*" + re.escape(param) + r'.*'
    new = "#define " + param + ' ' + str(value)
    content = ''
    with open("config.h", 'r') as f:
        content = f.read()
    content = re.sub(old, new, content)
    with open("config.h", 'w') as f:
        f.write(content)


def update_workload_type(w):
    update_config_file("WORKLOAD", w)


def update_algo_type(algo):
    update_config_file("CC_ALG", algo)


def run_tpcc_fixed_cores(algo, cores, sockets):
    print("%s" % algo)
    print("wharehouse\ttxns/sec")
    cores_per_socket = cores / (2 * sockets)
    with open(os.path.join(outdir, "tpcc-varying-wharehouse"), 'a') as f:
        f.write("# %s: %d cores\n" % (algo, cores))
        f.write("# wharehouse\ttxns/sec\n")
        for w in [x*cores_per_socket for x in range(1, sockets * 2 + 1)]:
            args = "./rundb -t%d -n%d" % (cores, w)
            output = run(args)[0].decode("utf-8")
            with open(os.path.join(outdir, "tpcc-algo-%s-c%d-w%d" %
                                   (algo, cores, w)), 'w') as f1:
                for i in output:
                    f1.write(i)
            txns_per_sec = get_stats(output, cores)
            f.write("%d\t%f\n" % (w, txns_per_sec))
            print("%d\t%f" % (w, txns_per_sec))


def run_tpcc_fixed_wharehouse(algo, cores, sockets):
    print("%s" % algo)
    print("Cores\ttxns/sec")
    cores_per_socket = cores / (2 * sockets)
    with open(os.path.join(outdir, "tpcc-varying-cores"), 'a') as f:
        f.write("# %s: %d wharehouse\n" % (algo, cores))
        f.write("# Cores\ttxns/sec\n")
        for c in [1] + [x*cores_per_socket for x in range(1, sockets * 2 + 1)]:
            args = "./rundb -t%d -n%d" % (c, sockets)
            output = run(args)
            print(output[1])
            output = output[0].decode("utf-8")
            with open(os.path.join(outdir, "tpcc-algo-%s-c%d-w%d" %
                                   (algo, c, sockets)), 'w') as f1:
                for i in output:
                    f1.write(i)
            txns_per_sec = get_stats(output, c)
            f.write("%d\t%f\n" % (c, txns_per_sec))
            print("%d\t%f" % (c, txns_per_sec))


def run_tpcc(cores, sockets, algo):
    update_workload_type("TPCC")
    for i in algo:
        update_algo_type(i)
        compile_code()
        # run 8 wharehouse with varying core count
        run_tpcc_fixed_wharehouse(i, cores, sockets)
        # run with fixed cores, i.e. at 240
        run_tpcc_fixed_cores(i, cores, sockets)


def run_ycsb_with_contention(algo, cores, sockets, th=0, rd=1, qpt=2,
                             hspot=0, mpart=0, ctype="no"):
    print("%s: %s contention" % (algo, ctype))
    print("Cores\ttxns/sec")
    cores_per_socket = cores / (2 * sockets)
    with open(os.path.join(outdir, "ycsb-%s-contention" % ctype), 'a') as f:
        f.write("# %s: YCSB %s contention \n" % (algo, ctype))
        f.write("# Cores\ttxns/sec\n")
        for c in [1] + [x*cores_per_socket for x in range(1, sockets * 2 + 1)]:
            args = "./rundb -t%d -c%d -e%d -r%f -w%f -z%f " \
                    "-s3225600 -R%d" % (c, hspot, mpart, rd, 1-rd, th, qpt)
            output = run(args)
            output = output[0].decode("utf-8")
            with open(os.path.join(outdir, "ycsb-ctn-%s-algo-%s-c%d" %
                                   (ctype, algo, c)), 'w') as f1:
                f1.write(output)
            (txns_per_sec, abrts) = get_stats(output, c)
            f.write("%d\t%f\t%d\n" % (c, txns_per_sec, abrts))
            print("%d\t%f\t%d" % (c, txns_per_sec, abrts))


def run_ycsb(cores, sockets, algo):
    update_workload_type("YCSB")
    for i in algo:
        update_algo_type(i)
        compile_code()
        # no contention
        run_ycsb_with_contention(i, cores, sockets, th=0, rd=1, qpt=2)
        # medium contention
        run_ycsb_with_contention(i, cores, sockets, th=0.6, rd=0.9, qpt=16,
                                 hspot=10, mpart=60, ctype="med")
        # high contention
        run_ycsb_with_contention(i, cores, sockets, th=0.8, rd=0.5, qpt=16,
                                 hspot=10, mpart=75, ctype="high")


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
    run_ycsb(cores, num_sockets, algo)
    run_tpcc(cores, num_sockets, algo)

if __name__ == '__main__':
    main()
