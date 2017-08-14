#!/bin/bash


btime=15000

#for update in 1 10 100 500
for update in 20 200 400 # 500
do
	mkdir -p output/${update}
	echo "update: ${update} logical"
	for core in 1 2 4 8 `seq 15 15 240`
	do
		for writes in 1 #2 5 10 20
		do
			v=`./bench-rlu-logical -a -b1000 -d${btime} -i100 -r200 -w${writes} -u${update} -n${core} | grep "#ops" | awk '{printf("%0.2lf\n", $3 / 1000000)}'`
			echo -e "${core}\t${v}"
		done
		# echo "update: ${update} core: ${core} others"
		# ./bench-hp-harris -a -b1000 -d${btime} -i100000 -r200000 -u${update} -n${core} | grep "#ops" | awk '{printf("%0.2lf\n", $3 / 1000000)}' >> output/harris.hp
		# ./bench-harris -a -b1000 -d${btime} -i100000 -r200000 -u${update} -n${core} | grep "#ops" | awk '{printf("%0.2lf\n", $3 / 1000000)}' >> output/harris
		# ./bench-rcu -a -b1000 -d${btime} -i100000 -r200000 -u${update} -n${core} | grep "#ops" | awk '{printf("%0.2lf\n", $3 / 1000000)}' >> output/rcu
	done
	echo "update: ${update} physical"
	for core in 1 2 4 8 `seq 15 15 240`
	do
		for writes in 1 #2 5 10 20
		do
			v=`./bench-rlu-phystmstmp -a -b1000 -d${btime} -i100 -r200 -w${writes} -u${update} -n${core} | grep "#ops" | awk '{printf("%0.2lf\n", $3 / 1000000)}'`
			echo -e "p: ${core}\t${v}"
		done
		# echo "update: ${update} core: ${core} others"
		# ./bench-hp-harris -a -b1000 -d${btime} -i100000 -r200000 -u${update} -n${core} | grep "#ops" | awk '{printf("%0.2lf\n", $3 / 1000000)}' >> output/harris.hp
		# ./bench-harris -a -b1000 -d${btime} -i100000 -r200000 -u${update} -n${core} | grep "#ops" | awk '{printf("%0.2lf\n", $3 / 1000000)}' >> output/harris
		# ./bench-rcu -a -b1000 -d${btime} -i100000 -r200000 -u${update} -n${core} | grep "#ops" | awk '{printf("%0.2lf\n", $3 / 1000000)}' >> output/rcu
	done
done
