#!/usr/bin/env python3

import os
import sys
from itertools import combinations
import collections

timestamp_list = []
diff_table = []
diff_socket_table = []
per_cpu_table = []
diff_freq_table = {0:0}
socket_diff_freq_table = []

def parse_files(dirname, files):
    for i in files:
        with open(os.path.join(dirname, i)) as f:
            data = f.readlines()
            t = []
            for d in data:
                t.append(int(d.split(' ')[1].strip('\n')))
            timestamp_list.append(t)


def get_max_diff(t, num_cores):
    v = sorted(t[1:num_cores])
    i = 0
    j = num_cores - 2
    # while i < j-1:
    #     if v[j] - v[i] <= 5000:
    #         break
    #     j -= 1
    return (v[j] - v[i])


def parse_timestamp_global():
    num_cores = len(timestamp_list[0])

    for t in timestamp_list:
        diff_table.append(get_max_diff(t, num_cores))

    # print(diff_table)
    print("global min is %d" % min(diff_table))


def __get_max_diff(t, start, end, eidx):
    v = sorted(t[start:end])
    return v[eidx-1] - v[0]


def get_max_diff_socket(t, s, num_cores):
    start = 1
    end = num_cores
    j = num_cores - 1
    if s != 0:
        start = s * num_cores
        end = start + num_cores
        j = num_cores
    return __get_max_diff(t, start, end, j)


def parse_timestamp_socket(num_sockets):
    num_cores = len(timestamp_list[0])
    num_cores_per_socket = int(num_cores / num_sockets)
    v = [[] for i in range(0, num_sockets)]
    for t in timestamp_list:
        for s in range(0, num_sockets):
            v[s].append(get_max_diff_socket(t, s, num_cores_per_socket))
    for s in range(0, num_sockets):
        print("socket: %d, diff: %d" % (s, min(v[s])))
        # with open("output-%d.txt" % s, "w") as f:
        #     for i in sorted(v[s]):
        #         f.write(str(i)+"\n")


def __update_diff_table(a, b, value):
    if b != a:
        if value in diff_freq_table:
            diff_freq_table[value] += 1
        else:
            diff_freq_table.update({value:1})

def __update_diff_table_socket(a, b, sid, value):
    if b != a:
        if value in socket_diff_freq_table[sid]:
            socket_diff_freq_table[sid][value] += 1
        else:
            socket_diff_freq_table[sid].update({value:1})


def build_table(a, b, sid):
    v = []
    for i in timestamp_list:
        value = i[b] - i[a]
        v.append((abs(value), value))
        __update_diff_table(a, b, value)
        __update_diff_table_socket(a, b, sid, value)
    v.sort(key=lambda x: x[0])
    print("%d - %d: %d" % (a, b, v[0][1]))


def build_per_cpu_table(num_sockets, ref_core):
    num_cores = len(timestamp_list[0])
    for i in range(1, num_cores):
        build_table(ref_core, i, int(i / (num_cores / num_sockets)))


def main():
    if (len(sys.argv) < 3):
        print("%s dirname < . > #sockets" % (sys.argv[0]))
        return -1

    dirname = sys.argv[1]
    num_sockets = int(sys.argv[2])
    for i in range(0, num_sockets):
        socket_diff_freq_table.append({})

    files = os.listdir(dirname)
    parse_files(dirname, files)

    parse_timestamp_global()
    parse_timestamp_socket(num_sockets)
    build_per_cpu_table(1, num_sockets)
    with open("freq-table-global.txt", "w") as f:
        for k, v in sorted(diff_freq_table.items()):
            if k >= -1000 and k <= 1000:
                f.write("%s, %s\n" % (str(k), str(v)))
    for i in range(0, num_sockets):
        with open("freq-table-socket-%d.txt" % i, "w") as f:
            for k, v in sorted(socket_diff_freq_table[i].items()):
                if k >= -1000 and k <= 1000:
                    f.write("%s\n" % (str(v)))

    num_cores = len(timestamp_list[0])
    for i in range(1, num_sockets):
        socket_diff_freq_table[i].clear()
        build_per_cpu_table(int(num_sockets), int(i * (num_cores / num_sockets)))
        with open("freq-table-socket-%d.txt" % i, "w") as f:
            for k, v in sorted(socket_diff_freq_table[i].items()):
                if k >= -1000 and k <= 1000:
                    f.write("%s\n" % (str(v)))


if __name__ == '__main__':
    main()
