#!/bin/bash


for u in 0 1 10 50 100
do
	for i in 1 `seq 15 15 240`
	do
		for s in 1024 8192 16384 32768
		do
			r=$(( $s * 2 ))
			v=`./bin/ESTM-hashtable -d 15000 -i${s} -r${r} -t$i -u${u} | grep "#txs" | awk '{print $4}' | sed -e 's/(//'`
			echo "$i\t$v" >> ht-update-${u}-size-${s}.dat
			echo "ht: core: $i u: $u s: $s $v"

			sleep 1
			v=`./bin/ESTM-linkedlist -d 15000 -i${s} -r${r} -t$i -u${u} | grep "#txs" | awk '{print $4}' | sed -e 's/(//'`
			echo "$i\t$v" >> ll-update-${u}-size-${s}.dat
			echo "ll: core: $i u: $u s: $s $v"

			sleep 1
			v=`./bin/ESTM-rbtree -d 15000 -i${s} -r${r} -t$i -u${u} | grep "#txs" | awk '{print $4}' | sed -e 's/(//'`
			echo "$i\t$v" >> rb-update-${u}-size-${s}.dat
			echo "rb: core: $i u: $u s: $s $v"

			sleep 1
			v=`./bin/ESTM-specfriendly-tree -d 15000 -i${s} -r${r} -t$i -u${u} | grep "#txs" | awk '{print $4}' | sed -e 's/(//'`
			echo "$i\t$v" >> st-update-${u}-size-${s}.dat
			echo "st: core: $i u: $u s: $s $v"

			sleep 1
		done
	done
done
