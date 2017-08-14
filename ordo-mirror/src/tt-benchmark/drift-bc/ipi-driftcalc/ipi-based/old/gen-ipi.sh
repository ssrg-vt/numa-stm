#!/bin/bash

# $1 --> total physical cpus - 1

sudo grep "ipi cost" /var/log/kern.log | tail -$1 | awk -F":" '{printf("%d\t%d\n",$5,$6)}'
