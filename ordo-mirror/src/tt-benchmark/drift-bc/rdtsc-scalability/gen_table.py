#!/usr/bin/env python2

import subprocess

from itertools import combinations

import gen_cpuseq

iters = 100


def sh(*args):
    out = subprocess.check_output(args)
    #return out.decode("utf8").strip()
    return out


def get_cores():
    return [int(cpu["processor"]) for cpu in gen_cpuseq.seq(gen_cpuseq.cpuinfo)]


def build_table(a, b):
    print "sudo ./binaries/dual-offset %d %d 100000" % (a, b)
    tok = sh("sudo", "./binaries/dual-offset", str(a), str(b), "100000")
    with open("output/%d-%d" % (a,b), "w") as f:
        f.write(tok)

m = {}
for (a, b) in combinations(get_cores(), 2):
    key = (min(a, b), max(a, b))
    build_table(*key)
