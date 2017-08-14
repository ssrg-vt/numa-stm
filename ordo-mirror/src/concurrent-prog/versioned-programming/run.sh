#!/bin/bash

for u in 100 200 400
do
	echo "physical: $u"
	for i in 1 2 8 `seq 15 15 240`
	do
		v=`./bench-rlu-phys -d15000 -n$i -i100000 -r200000 -u$u | grep ops | awk '{print $4}'`
		echo -e "$i\t$v" >> output.phy
		echo -e "$i\t$v"
	done
	echo "logical: $u"
	for i in 1 2 8 `seq 15 15 240`
	do
		v=`./bench-rlu-log -d15000 -n$i -i100000 -r200000 -u$u | grep ops | awk '{print $4}'`
		echo -e "$i\t$v" >> output.log
		echo -e "$i\t$v"
	done
done
