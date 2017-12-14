
#-----------------------------------------------------------------------------------------------------------------------

echo "Bayes"
echo
:>"bayes.csv"
for i in 1 2 4 8
do

	ringsw_time=0
	NOrecRH_time=0
	tl2_time=0
	htm_time=0
	part_time=0

for (( j = 1 ; j <= 1; j++ ))
do

	export STM_CONFIG=RingSW
	temp=$(echo "$(/home/mohamedin/Downloads/orig_rstm_2/rstm_build/stamp-0.9.10/bayes/bayesSTM64 -v32 -r4096 -n10 -p40 -i2 -e8 -s1 -t$i)" | egrep -e 'Learn time[ ]*=' | egrep -e [1234567890.]* -o)
	ringsw_time=$(echo $ringsw_time $temp | awk ' { printf "%f\n", $1 + $2 } ') 

	export STM_CONFIG=NOrecRH
	temp=$(echo "$(/home/mohamedin/Downloads/rstm_rh/rstm_build/stamp-0.9.10/bayes/bayesSTM64 -v32 -r4096 -n10 -p40 -i2 -e8 -s1 -t$i)" | egrep -e 'Learn time[ ]*=' | egrep -e [1234567890.]* -o)
	NOrecRH_time=$(echo $NOrecRH_time $temp | awk ' { printf "%f\n", $1 + $2 } ') 

	export STM_CONFIG=LLT
	temp=$(echo "$(/home/mohamedin/Downloads/orig_rstm_2/rstm_build/stamp-0.9.10/bayes/bayesSTM64 -v32 -r4096 -n10 -p40 -i2 -e8 -s1 -t$i)" | egrep -e 'Learn time[ ]*=' | egrep -e [1234567890.]* -o)
	tl2_time=$(echo $tl2_time $temp | awk ' { printf "%f\n", $1 + $2 } ') 

	temp=$(echo "$(/home/mohamedin/stamp-tsx/bayes/bayes -v32 -r4096 -n10 -p40 -i2 -e8 -s1 -t$i)" | egrep -e 'Learn time[ ]*=' | egrep -e [1234567890.]* -o)
	htm_time=$(echo $htm_time $temp | awk ' { printf "%f\n", $1 + $2 } ') 


	temp=$(echo "$(/home/mohamedin/git/ft-stm/STAMP/bayes/bayes -v32 -r4096 -n10 -p40 -i2 -e8 -s1 -t$i)" | egrep -e 'Learn time[ ]*=' | egrep -e [1234567890.]* -o)
	part_time=$(echo $part_time $temp | awk ' { printf "%f\n", $1 + $2 } ') 
	
done
	
	ringsw_time=$(echo $ringsw_time 1.0 | awk ' { printf "%f\n", $1 / $2 } ') 
	NOrecRH_time=$(echo $NOrecRH_time 1.0 | awk ' { printf "%f\n", $1 / $2 } ')
	tl2_time=$(echo $tl2_time 1.0 | awk ' { printf "%f\n", $1 / $2 } ')
	htm_time=$(echo $htm_time 1.0 | awk ' { printf "%f\n", $1 / $2 } ')
	part_time=$(echo $part_time 1.0 | awk ' { printf "%f\n", $1 / $2 } ')	

	echo "Number of threads: " $i
	echo "RingSTM: " $ringsw_time
	echo "NOrecRH: " $NOrecRH_time
	echo "TL2: " $tl2_time
	echo "HTM: " $htm_time
	echo "Part: " $part_time
	echo

	echo $i $ringsw_time $NOrecRH_time $tl2_time $htm_time $part_time >> "bayes.csv"
	
done

#-----------------------------------------------------------------------------------------------------------------------
