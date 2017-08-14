#!/bin/bash

for core in 1 2 4 8 `seq 15 15 120`
do
	for update in 0 1 10 20 50 100 200 400 500 1000
	do
		for writes in 1 2 10 20
		do
			echo "writes: ${writes} update: ${update} core: ${core}"
			task_core=$(( $core - 1))
			if [[ "$update" -eq 0 ]]
			then
				if [[ "$writes" -eq 1 ]]
				then
					taskset -c 0-${task_core} ./bench-rcu -a -g1 -b8192 -d15000 -i65536 -r131072 -w${writes} -u${update} -n${core} | grep "#ops" | awk '{printf("%0.2lf\n", $3 / 1000000)}' >> output/rcu.update-${update}
				fi
			fi
			sleep 0.5
			taskset -c 0-${task_core} ./bench-rlu-log -a -g1 -b8192 -d15000 -i65536 -r131072 -w${writes} -u${update} -n${core} | grep "#ops" | awk '{printf("%0.2lf\n", $3 / 1000000)}' >> output/rlu.logical.writes-${writes}.update-${update}
			sleep 0.5
			taskset -c 0-${task_core} ./bench-rlu-phys -a -g1 -b8192 -d15000 -i65536 -r131072 -w${writes} -u${update} -n${core} | grep "#ops" | awk '{printf("%0.2lf\n", $3 / 1000000)}' >> output/rlu.phys.writes-${writes}.update-${update}
		done
	done
done
