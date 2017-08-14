#!/usr/bin/env python3

import sys
import subprocess
import shlex
import re
import numpy


core_set = [1, 2, 4, 8] + [x * 15 for x in range(1, 17)]

bench = {
    "bayes": ["./bayes/bayes -v32 -r4096 -n10 -p40 -i2 -e9 -s1", "-t",
              "(?<=Learn time = )[0-9]*\.[0-9]*"],
    "genome": ["./genome/genome -g65536 -s256 -n33554432", "-t",
               "(?<=Time = )[0-9]*\.[0-9]*"],
    "intruder": ["./intruder/intruder -a10 -l2048 -n10000 -s1", "-t",
                 "(?<=Elapsed time )*[0-9]*\.[0-9]*"],
    "kmeans": ["./kmeans/kmeans -m160 -n160 -t0.01 -i "
               "kmeans/inputs/random-n262144-d32-c16.txt", "-p",
               "(?<=Time: )[0-9]*\.[0-9]*"],
    "labyrinth": ["./labyrinth/labyrinth -i "
                  "labyrinth/inputs/random-x1024-y1024-z15-n64.txt", "-t",
                  "(?<=Elapsed time )*[0-9]*\.[0-9]*"],
    "ssca2": ["./ssca2/ssca2 -s21 -i1.0 -u1.0 -l3 -p3", "-t",
              "(?<=Time taken for all is )[0-9]*\.[0-9]*"],
    "vacation": ["./vacation/vacation -n2 -q90 -u98 -r1048576 -t4194304", "-c",
                 "(?<=Time = )[0-9]*\.[0-9]*"],
    "vacation-hi": ["./vacation/vacation -n4 -q90 -u90 -r184857 -t12194304",
                    "-c", "(?<=Time = )[0-9]*\.[0-9]*"],
    # "yada": ["./yada/yada -a15 -i yada/inputs/ttimeu1000000.2", "-t",
    # ""]
}


def run(args):
    args = shlex.split(args)
    p = subprocess.Popen(args, stdout=subprocess.PIPE,
                         stderr=subprocess.PIPE)
    return p.communicate()


def compile_code(benchmark):
    run("make -C %s -f Makefile.stm clean" % benchmark)
    run("make -C %s -f Makefile.stm" % benchmark)


def run_bench(stm_name, benchname, blist, core, ntrial):
    output = ""
    with open("%s.%s" % (stm_name, benchname), "a") as f1:
        f1.write("==\tcore: %d trial: %d\t ==\n" % (core, ntrial))
        args = "taskset -c 0-%d %s %s%d" % (core - 1, blist[0], blist[1], core)
        output = run(args)[0].decode("utf-8")
        for i in output:
            f1.write(i)
        f1.write("\n\n")
    m = re.search(blist[2], output)
    if m is None:
        print("Rerunning it, didn't get the value for this run: %d" % ntrial)
        return run_bench(stm_name, benchname, blist, core, ntrial)
    else:
        return float(m.group(0))


def main():
    if (len(sys.argv) < 3):
        print("%s name<tl2/tinystm trials" % (sys.argv[0]))
        exit(-1)
    filename = "results.%s" % sys.argv[1]
    trials = int(sys.argv[2])
    with open(filename, "a") as f:
        for k, v in sorted(bench.items()):
            compile_code(k)
            f.write("== %s ==\n" % k)
            print("== %s ==\n" % k)
            for core in core_set:
                rtimes = []
                for i in range(1, trials+1):
                    rtimes.append(run_bench(sys.argv[1], k, v, core, i))
                    print("%f\t" % (rtimes[i-1]), end='', flush=True)
                print("")
                np_arry = numpy.array(rtimes)
                f.write("%d\t%f\t%f\t%f\t%f\n" % (core, numpy.average(np_arry),
                                                  min(rtimes), max(rtimes),
                                                  numpy.std(np_arry)))
                print("%d\t%f\t%f\t%f\t%f\n" % (core, numpy.average(np_arry),
                                                min(rtimes), max(rtimes),
                                                numpy.std(np_arry)))
            f.write("\n\n")

if __name__ == '__main__':
    main()
