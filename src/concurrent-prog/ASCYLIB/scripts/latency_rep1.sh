#!/bin/bash

cores=$1;
shift;

reps=$1;
shift;

source scripts/lock_exec;
source scripts/config;

result_type=$1;

if [ "$result_type" = "max" ];
then
    run_script="./scripts/run_rep_lat_max.sh $reps";
    echo "# Result from $reps repetitions: max";
    shift;

elif [ "$result_type" = "min" ];
then
    run_script="./scripts/run_rep_lat_min.sh $reps";
    echo "# Result from $reps repetitions: min";
    shift;
elif [ "$result_type" = "median" ];
then
    run_script="./scripts/run_rep_lat_med.sh $reps";
    echo "# Result from $reps repetitions: median";
    shift;
else
    run_script="./scripts/run_rep_lat_max.sh $reps";
    echo "# Result from $reps repetitions: max (default). Available: min, max, median";
fi;

prog1="$1";
shift;
params="$@";

printf "#   %-60s\n" "$prog1" 

echo "#co get_s  get_f put_s  put_f rem_s";

d=0;
for c in 1 $cores
do
    if [ $c -eq 1 ];
    then
	if [ $d -eq 1 ];
	then
	    continue;
	fi;
    fi;
    d=1;

    printf "%-4d" $c;
    prog=$prog1;

    thr=$($run_script ./$prog $params -n$c);
    printf "%-10d%-10d%-10d%-10d%-10d%-10d" $thr;

    echo "";
done;

source scripts/unlock_exec;
