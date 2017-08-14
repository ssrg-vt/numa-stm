#!/bin/bash

# $1: start
# $2: gap
# $3: max

for i in 1 2  `seq $1 $2 $3`; do v=`./binaries/oplogrmap $i 1000000 | awk '{printf("%d\n", $11)}'`; d=`echo "$i * $v" | bc`; echo -e "$i\t$d"; sleep 1; done
for i in 1 2  `seq $1 $2 $3`; do v=`./binaries/rwspinlockrmap $i 1000000 | awk '{printf("%d\n", $11)}'`; d=`echo "$i * $v" | bc`; echo -e "$i\t$d"; sleep 1; done
