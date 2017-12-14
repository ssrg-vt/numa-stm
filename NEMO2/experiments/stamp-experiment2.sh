echo "Kmeans Hi"
echo
:>"kmeansHi.csv"
for i in 1 2 4 8
do

	ringsw_time=0
	NOrec_time=0
	tl2_time=0
	htm_time=0
	part_time=0
	
for (( j = 1 ; j <= 5; j++ ))
do

	export STM_CONFIG=RingSW
	temp=$(echo "$(/home/mohamedin/Downloads/orig_rstm_2/rstm_build/stamp-0.9.10/kmeans/kmeansSTM64 -m15 -n15 -t0.00001 -i experiments/inputs/random-n65536-d32-c16.txt -p$i)" | egrep -e 'Time: [^ ]*' -o | cut -d ' ' -f 2)
	ringsw_time=$(echo $ringsw_time $temp | awk ' { printf "%f\n", $1 + $2 } ') 

	export STM_CONFIG=NOrecRH
	temp=$(echo "$(/home/mohamedin/Downloads/rstm_rh/rstm_build/stamp-0.9.10/kmeans/kmeansSTM64 -m15 -n15 -t0.00001 -i experiments/inputs/random-n65536-d32-c16.txt -p$i)" | egrep -e 'Time: [^ ]*' -o | cut -d ' ' -f 2)
	NOrec_time=$(echo $NOrec_time $temp | awk ' { printf "%f\n", $1 + $2 } ') 

	export STM_CONFIG=LLT
	temp=$(echo "$(/home/mohamedin/Downloads/orig_rstm_2/rstm_build/stamp-0.9.10/kmeans/kmeansSTM64 -m15 -n15 -t0.00001 -i experiments/inputs/random-n65536-d32-c16.txt -p$i)" | egrep -e 'Time: [^ ]*' -o | cut -d ' ' -f 2)
	tl2_time=$(echo $tl2_time $temp | awk ' { printf "%f\n", $1 + $2 } ') 


	temp=$(echo "$(/home/mohamedin/stamp-tsx/kmeans/kmeans -m15 -n15 -t0.00001 -i experiments/inputs/random-n65536-d32-c16.txt -p$i)" | egrep -e 'Time: [^ ]*' -o | cut -d ' ' -f 2)
	htm_time=$(echo $htm_time $temp | awk ' { printf "%f\n", $1 + $2 } ') 


	temp=$(echo "$(/home/mohamedin/git/ft-stm/STAMP/kmeans/kmeans -m15 -n15 -t0.00001 -i experiments/inputs/random-n65536-d32-c16.txt -p$i)" | egrep -e 'Time: [^ ]*' -o | cut -d ' ' -f 2)
	part_time=$(echo $part_time $temp | awk ' { printf "%f\n", $1 + $2 } ') 


done
	
	ringsw_time=$(echo $ringsw_time 5.0 | awk ' { printf "%f\n", $1 / $2 } ') 
	NOrec_time=$(echo $NOrec_time 5.0 | awk ' { printf "%f\n", $1 / $2 } ')
	tl2_time=$(echo $tl2_time 5.0 | awk ' { printf "%f\n", $1 / $2 } ')
	htm_time=$(echo $htm_time 5.0 | awk ' { printf "%f\n", $1 / $2 } ')
	part_time=$(echo $part_time 5.0 | awk ' { printf "%f\n", $1 / $2 } ')
	
	echo "Number of threads: " $i
	echo "RingSTM: " $ringsw_time
	echo "NOrec: " $NOrec_time
	echo "TL2: " $tl2_time
	echo "HTM: " $htm_time
	echo "Part: " $part_time
	echo

	echo $i $ringsw_time $NOrec_time $tl2_time $htm_time $part_time >> "kmeansHi.csv"
	
done

#-----------------------------------------------------------------------------------------------------------------------

echo "Kmeans Low"
echo
:>"kmeansLow.csv"
for i in 1 2 4 8
do

	ringsw_time=0
	NOrec_time=0
	tl2_time=0
	htm_time=0
	part_time=0

for (( j = 1 ; j <= 5; j++ ))
do

	export STM_CONFIG=RingSW
	temp=$(echo "$(/home/mohamedin/Downloads/orig_rstm_2/rstm_build/stamp-0.9.10/kmeans/kmeansSTM64 -m40 -n40 -t0.00001 -i experiments/inputs/random-n65536-d32-c16.txt -p$i)" | egrep -e 'Time: [^ ]*' -o | cut -d ' ' -f 2)
	ringsw_time=$(echo $ringsw_time $temp | awk ' { printf "%f\n", $1 + $2 } ') 

	export STM_CONFIG=NOrecRH
	temp=$(echo "$(/home/mohamedin/Downloads/rstm_rh/rstm_build/stamp-0.9.10/kmeans/kmeansSTM64 -m40 -n40 -t0.00001 -i experiments/inputs/random-n65536-d32-c16.txt -p$i)" | egrep -e 'Time: [^ ]*' -o | cut -d ' ' -f 2)
	NOrec_time=$(echo $NOrec_time $temp | awk ' { printf "%f\n", $1 + $2 } ') 

	export STM_CONFIG=LLT
	temp=$(echo "$(/home/mohamedin/Downloads/orig_rstm_2/rstm_build/stamp-0.9.10/kmeans/kmeansSTM64 -m40 -n40 -t0.00001 -i experiments/inputs/random-n65536-d32-c16.txt -p$i)" | egrep -e 'Time: [^ ]*' -o | cut -d ' ' -f 2)
	tl2_time=$(echo $tl2_time $temp | awk ' { printf "%f\n", $1 + $2 } ') 

	temp=$(echo "$(/home/mohamedin/stamp-tsx/kmeans/kmeans -m40 -n40 -t0.00001  -i experiments/inputs/random-n65536-d32-c16.txt -p$i)" | egrep -e 'Time: [^ ]*' -o | cut -d ' ' -f 2)
	htm_time=$(echo $htm_time $temp | awk ' { printf "%f\n", $1 + $2 } ') 


	temp=$(echo "$(/home/mohamedin/git/ft-stm/STAMP/kmeans/kmeans -m40 -n40 -t0.00001  -i experiments/inputs/random-n65536-d32-c16.txt -p$i)" | egrep -e 'Time: [^ ]*' -o | cut -d ' ' -f 2)
	part_time=$(echo $part_time $temp | awk ' { printf "%f\n", $1 + $2 } ') 

done
	
	ringsw_time=$(echo $ringsw_time 5.0 | awk ' { printf "%f\n", $1 / $2 } ') 
	NOrec_time=$(echo $NOrec_time 5.0 | awk ' { printf "%f\n", $1 / $2 } ')
	tl2_time=$(echo $tl2_time 5.0 | awk ' { printf "%f\n", $1 / $2 } ')
	htm_time=$(echo $htm_time 5.0 | awk ' { printf "%f\n", $1 / $2 } ')
	part_time=$(echo $part_time 5.0 | awk ' { printf "%f\n", $1 / $2 } ')

	echo "Number of threads: " $i
	echo "RingSTM: " $ringsw_time
	echo "NOrec: " $NOrec_time
	echo "TL2: " $tl2_time
	echo "HTM: " $htm_time
	echo "Part: " $part_time
	echo

	echo $i $ringsw_time $NOrec_time $tl2_time $htm_time $part_time >> "kmeansLow.csv"
	
done

#-----------------------------------------------------------------------------------------------------------------------


echo "ssca2"
echo
:>"ssca2.csv"
for i in 1 2 4 8 
do

	ringsw_time=0
	NOrec_time=0
	tl2_time=0
	htm_time=0
	part_time=0	

for (( j = 1 ; j <= 5; j++ ))
do

	export STM_CONFIG=RingSW
	temp=$(echo "$(/home/mohamedin/Downloads/orig_rstm_2/rstm_build/stamp-0.9.10/ssca2/ssca2STM64 -s20 -i1.0 -u1.0 -l3 -p3 -t$i)" | egrep -e 'Time taken for all is' | egrep -e [1234567890.]* -o)
	ringsw_time=$(echo $ringsw_time $temp | awk ' { printf "%f\n", $1 + $2 } ') 

	export STM_CONFIG=NOrecRH
	temp=$(echo "$(/home/mohamedin/Downloads/rstm_rh/rstm_build/stamp-0.9.10/ssca2/ssca2STM64 -s20 -i1.0 -u1.0 -l3 -p3 -t$i)" | egrep -e 'Time taken for all is' | egrep -e [1234567890.]* -o)
	NOrec_time=$(echo $NOrec_time $temp | awk ' { printf "%f\n", $1 + $2 } ') 

	export STM_CONFIG=LLT
	temp=$(echo "$(/home/mohamedin/Downloads/orig_rstm_2/rstm_build/stamp-0.9.10/ssca2/ssca2STM64 -s20 -i1.0 -u1.0 -l3 -p3 -t$i)" | egrep -e 'Time taken for all is' | egrep -e [1234567890.]* -o)
	tl2_time=$(echo $tl2_time $temp | awk ' { printf "%f\n", $1 + $2 } ') 

	temp=$(echo "$(/home/mohamedin/stamp-tsx/ssca2/ssca2 -s20 -i1.0 -u1.0 -l3 -p3 -t$i)"  | egrep -e 'Time taken for all is' | egrep -e [1234567890.]* -o)
	htm_time=$(echo $htm_time $temp | awk ' { printf "%f\n", $1 + $2 } ') 


	temp=$(echo "$(/home/mohamedin/git/ft-stm/STAMP/ssca2/ssca2 -s20 -i1.0 -u1.0 -l3 -p3 -t$i)"  | egrep -e 'Time taken for all is' | egrep -e [1234567890.]* -o)
	part_time=$(echo $part_time $temp | awk ' { printf "%f\n", $1 + $2 } ') 

done

	ringsw_time=$(echo $ringsw_time 5.0 | awk ' { printf "%f\n", $1 / $2 } ') 
	NOrec_time=$(echo $NOrec_time 5.0 | awk ' { printf "%f\n", $1 / $2 } ')
	tl2_time=$(echo $tl2_time 5.0 | awk ' { printf "%f\n", $1 / $2 } ')
	htm_time=$(echo $htm_time 5.0 | awk ' { printf "%f\n", $1 / $2 } ')
	part_time=$(echo $part_time 5.0 | awk ' { printf "%f\n", $1 / $2 } ')

	echo "Number of threads: " $i
	echo "RingSTM: " $ringsw_time
	echo "NOrec: " $NOrec_time
	echo "TL2: " $tl2_time
	echo "HTM: " $htm_time
	echo "Part: " $part_time
	echo

	echo $i $ringsw_time $NOrec_time $tl2_time $htm_time $part_time >> "ssca2.csv"
	
done

#-----------------------------------------------------------------------------------------------------------------------

echo "labyrinth"
echo
:>"labyrinth.csv"
for i in 1 2 4 8 
do

	ringsw_time=0
	NOrec_time=0
	tl2_time=0
	htm_time=0
	part_time=0

for (( j = 1 ; j <= 5; j++ ))
do	

	export STM_CONFIG=RingSW
	temp=$(echo "$(/home/mohamedin/Downloads/orig_rstm_2/rstm_build/stamp-0.9.10/labyrinth/labyrinthSTM64 -i experiments/inputs/random-x512-y512-z7-n512.txt -t$i)" | egrep -e 'Elapsed time    = [^ ]*' -o | cut -d '=' -f 2 | cut -d ' ' -f 2)
	ringsw_time=$(echo $ringsw_time $temp | awk ' { printf "%f\n", $1 + $2 } ') 

	export STM_CONFIG=NOrecRH
	temp=$(echo "$(/home/mohamedin/Downloads/rstm_rh/rstm_build/stamp-0.9.10/labyrinth/labyrinthSTM64 -i experiments/inputs/random-x512-y512-z7-n512.txt -t$i)" | egrep -e 'Elapsed time    = [^ ]*' -o | cut -d '=' -f 2 | cut -d ' ' -f 2)
	NOrec_time=$(echo $NOrec_time $temp | awk ' { printf "%f\n", $1 + $2 } ') 

	export STM_CONFIG=LLT
	temp=$(echo "$(/home/mohamedin/Downloads/orig_rstm_2/rstm_build/stamp-0.9.10/labyrinth/labyrinthSTM64 -i experiments/inputs/random-x512-y512-z7-n512.txt -t$i)" | egrep -e 'Elapsed time    = [^ ]*' -o | cut -d '=' -f 2 | cut -d ' ' -f 2)
	tl2_time=$(echo $tl2_time $temp | awk ' { printf "%f\n", $1 + $2 } ') 

	temp=$(echo "$(/home/mohamedin/stamp-tsx/labyrinth/labyrinth -i experiments/inputs/random-x512-y512-z7-n512.txt -t$i)" | egrep -e 'Elapsed time    = [^ ]*' -o | cut -d '=' -f 2 | cut -d ' ' -f 2)
	htm_time=$(echo $htm_time $temp | awk ' { printf "%f\n", $1 + $2 } ') 


	temp=$(echo "$(/home/mohamedin/git/ft-stm/STAMP/labyrinth/labyrinth -i experiments/inputs/random-x512-y512-z7-n512.txt -t$i)"  | egrep -e 'Elapsed time    = [^ ]*' -o | cut -d '=' -f 2 | cut -d ' ' -f 2)
	part_time=$(echo $part_time $temp | awk ' { printf "%f\n", $1 + $2 } ') 

done
	
	ringsw_time=$(echo $ringsw_time 5.0 | awk ' { printf "%f\n", $1 / $2 } ') 
	NOrec_time=$(echo $NOrec_time 5.0 | awk ' { printf "%f\n", $1 / $2 } ')
	tl2_time=$(echo $tl2_time 5.0 | awk ' { printf "%f\n", $1 / $2 } ')
	htm_time=$(echo $htm_time 5.0 | awk ' { printf "%f\n", $1 / $2 } ')
	part_time=$(echo $part_time 5.0 | awk ' { printf "%f\n", $1 / $2 } ')

	
	echo "Number of threads: " $i
	echo "RingSTM: " $ringsw_time
	echo "NOrec: " $NOrec_time
	echo "TL2: " $tl2_time
	echo "HTM: " $htm_time
	echo "Part: " $part_time
	echo

	echo $i $ringsw_time $NOrec_time $tl2_time $htm_time $part_time >> "labyrinth.csv"
	
done

#-----------------------------------------------------------------------------------------------------------------------

echo "Vacation High"
echo
:>"vacationHi.csv"
for i in 1 2 4 8
do

	ringsw_time=0
	NOrec_time=0
	tl2_time=0
	htm_time=0
	part_time=0

for (( j = 1 ; j <= 5; j++ ))
do

	export STM_CONFIG=RingSW
	temp=$(echo "$(/home/mohamedin/Downloads/orig_rstm_2/rstm_build/stamp-0.9.10/vacation/vacationSTM64 -n4 -q60 -u90 -r1048576 -t4194304 -c$i)" | egrep -e 'Time = [^ ]*' -o | cut -d ' ' -f 3)
	ringsw_time=$(echo $ringsw_time $temp | awk ' { printf "%f\n", $1 + $2 } ') 

	export STM_CONFIG=NOrecRH
	temp=$(echo "$(/home/mohamedin/Downloads/rstm_rh/rstm_build/stamp-0.9.10/vacation/vacationSTM64 -n4 -q60 -u90 -r1048576 -t4194304 -c$i)" | egrep -e 'Time = [^ ]*' -o | cut -d ' ' -f 3)
	NOrec_time=$(echo $NOrec_time $temp | awk ' { printf "%f\n", $1 + $2 } ') 

	export STM_CONFIG=LLT
	temp=$(echo "$(/home/mohamedin/Downloads/orig_rstm_2/rstm_build/stamp-0.9.10/vacation/vacationSTM64 -n4 -q60 -u90 -r1048576 -t4194304 -c$i)" | egrep -e 'Time = [^ ]*' -o | cut -d ' ' -f 3)
	tl2_time=$(echo $tl2_time $temp | awk ' { printf "%f\n", $1 + $2 } ') 

	temp=$(echo "$(/home/mohamedin/stamp-tsx/vacation/vacation -n4 -q60 -u90 -r1048576 -t4194304 -c$i)" | egrep -e 'Time = [^ ]*' -o | cut -d ' ' -f 3)
	htm_time=$(echo $htm_time $temp | awk ' { printf "%f\n", $1 + $2 } ') 


	temp=$(echo "$(/home/mohamedin/git/ft-stm/STAMP/vacation/vacation -n4 -q60 -u90 -r1048576 -t4194304 -c$i)" | egrep -e 'Time = [^ ]*' -o | cut -d ' ' -f 3)
	part_time=$(echo $part_time $temp | awk ' { printf "%f\n", $1 + $2 } ') 

done
	
	ringsw_time=$(echo $ringsw_time 5.0 | awk ' { printf "%f\n", $1 / $2 } ') 
	NOrec_time=$(echo $NOrec_time 5.0 | awk ' { printf "%f\n", $1 / $2 } ')
	tl2_time=$(echo $tl2_time 5.0 | awk ' { printf "%f\n", $1 / $2 } ')
	htm_time=$(echo $htm_time 5.0 | awk ' { printf "%f\n", $1 / $2 } ')
	part_time=$(echo $part_time 5.0 | awk ' { printf "%f\n", $1 / $2 } ')	

	echo "Number of threads: " $i
	echo "RingSTM: " $ringsw_time
	echo "NOrec: " $NOrec_time
	echo "TL2: " $tl2_time
	echo "HTM: " $htm_time
	echo "Part: " $part_time
	echo

	echo $i $ringsw_time $NOrec_time $tl2_time $htm_time $part_time  >> "vacationHi.csv"
	
done

#-----------------------------------------------------------------------------------------------------------------------

echo "Vacation Low"
echo
:>"vacationLow.csv"
for i in 1 2 4 8
do

	ringsw_time=0
	NOrec_time=0
	tl2_time=0
	htm_time=0
	part_time=0

for (( j = 1 ; j <= 5; j++ ))
do

	export STM_CONFIG=RingSW
	temp=$(echo "$(/home/mohamedin/Downloads/orig_rstm_2/rstm_build/stamp-0.9.10/vacation/vacationSTM64 -n2 -q90 -u98 -r1048576 -t4194304 -c$i)" | egrep -e 'Time = [^ ]*' -o | cut -d ' ' -f 3)
	ringsw_time=$(echo $ringsw_time $temp | awk ' { printf "%f\n", $1 + $2 } ') 

	export STM_CONFIG=NOrecRH
	temp=$(echo "$(/home/mohamedin/Downloads/rstm_rh/rstm_build/stamp-0.9.10/vacation/vacationSTM64 -n2 -q90 -u98 -r1048576 -t4194304 -c$i)" | egrep -e 'Time = [^ ]*' -o | cut -d ' ' -f 3)
	NOrec_time=$(echo $NOrec_time $temp | awk ' { printf "%f\n", $1 + $2 } ') 

	export STM_CONFIG=LLT
	temp=$(echo "$(/home/mohamedin/Downloads/orig_rstm_2/rstm_build/stamp-0.9.10/vacation/vacationSTM64 -n2 -q90 -u98 -r1048576 -t4194304 -c$i)" | egrep -e 'Time = [^ ]*' -o | cut -d ' ' -f 3)
	tl2_time=$(echo $tl2_time $temp | awk ' { printf "%f\n", $1 + $2 } ') 

	temp=$(echo "$(/home/mohamedin/stamp-tsx/vacation/vacation -n2 -q90 -u98 -r1048576 -t4194304 -c$i)" | egrep -e 'Time = [^ ]*' -o | cut -d ' ' -f 3)
	htm_time=$(echo $htm_time $temp | awk ' { printf "%f\n", $1 + $2 } ') 


	temp=$(echo "$(/home/mohamedin/git/ft-stm/STAMP/vacation/vacation -n2 -q90 -u98 -r1048576 -t4194304 -c$i)" | egrep -e 'Time = [^ ]*' -o | cut -d ' ' -f 3)
	part_time=$(echo $part_time $temp | awk ' { printf "%f\n", $1 + $2 } ') 

done
	
	ringsw_time=$(echo $ringsw_time 5.0 | awk ' { printf "%f\n", $1 / $2 } ') 
	NOrec_time=$(echo $NOrec_time 5.0 | awk ' { printf "%f\n", $1 / $2 } ')
	tl2_time=$(echo $tl2_time 5.0 | awk ' { printf "%f\n", $1 / $2 } ')
	htm_time=$(echo $htm_time 5.0 | awk ' { printf "%f\n", $1 / $2 } ')
	part_time=$(echo $part_time 5.0 | awk ' { printf "%f\n", $1 / $2 } ')	

	echo "Number of threads: " $i
	echo "RingSTM: " $ringsw_time
	echo "NOrec: " $NOrec_time
	echo "TL2: " $tl2_time
	echo "HTM: " $htm_time
	echo "Part: " $part_time
	echo

	echo $i $ringsw_time $NOrec_time $tl2_time $htm_time $part_time  >> "vacationLow.csv"
	
done

#-----------------------------------------------------------------------------------------------------------------------

echo "Yada"
echo
:>"yada.csv"
for i in 1 2 4 8
do

	ringsw_time=0
	NOrec_time=0
	tl2_time=0
	htm_time=0
	part_time=0

for (( j = 1 ; j <= 5; j++ ))
do

	export STM_CONFIG=RingSW
	temp=$(echo "$(/home/mohamedin/Downloads/orig_rstm_2/rstm_build/stamp-0.9.10/yada/yadaSTM64 -a15 -i experiments/inputs/ttimeu1000000.2 -t$i)" | egrep -e 'Elapsed time[ ]*= [^ ]*' -o | egrep -e [1234567890.]* -o)
	ringsw_time=$(echo $ringsw_time $temp | awk ' { printf "%f\n", $1 + $2 } ') 

	export STM_CONFIG=NOrecRH
	temp=$(echo "$(/home/mohamedin/Downloads/rstm_rh/rstm_build/stamp-0.9.10/yada/yadaSTM64 -a15 -i experiments/inputs/ttimeu1000000.2 -t$i)" | egrep -e 'Elapsed time[ ]*= [^ ]*' -o | egrep -e [1234567890.]* -o)
	NOrec_time=$(echo $NOrec_time $temp | awk ' { printf "%f\n", $1 + $2 } ') 

	export STM_CONFIG=LLT
	temp=$(echo "$(/home/mohamedin/Downloads/orig_rstm_2/rstm_build/stamp-0.9.10/yada/yadaSTM64 -a15 -i experiments/inputs/ttimeu1000000.2 -t$i)" | egrep -e 'Elapsed time[ ]*= [^ ]*' -o | egrep -e [1234567890.]* -o)
	tl2_time=$(echo $tl2_time $temp | awk ' { printf "%f\n", $1 + $2 } ') 

	temp=$(echo "$(/home/mohamedin/stamp-tsx/yada/yada -a15 -i experiments/inputs/ttimeu1000000.2 -t$i)" | egrep -e 'Elapsed time[ ]*= [^ ]*' -o | egrep -e [1234567890.]* -o)
	htm_time=$(echo $htm_time $temp | awk ' { printf "%f\n", $1 + $2 } ') 


	temp=$(echo "$(/home/mohamedin/git/ft-stm/STAMP/yada/yada -a15 -i experiments/inputs/ttimeu1000000.2 -t$i)" | egrep -e 'Elapsed time[ ]*= [^ ]*' -o | egrep -e [1234567890.]* -o)
	part_time=$(echo $part_time $temp | awk ' { printf "%f\n", $1 + $2 } ') 

done
	
	ringsw_time=$(echo $ringsw_time 5.0 | awk ' { printf "%f\n", $1 / $2 } ') 
	NOrec_time=$(echo $NOrec_time 5.0 | awk ' { printf "%f\n", $1 / $2 } ')
	tl2_time=$(echo $tl2_time 5.0 | awk ' { printf "%f\n", $1 / $2 } ')
	htm_time=$(echo $htm_time 5.0 | awk ' { printf "%f\n", $1 / $2 } ')
	part_time=$(echo $part_time 5.0 | awk ' { printf "%f\n", $1 / $2 } ')	

	echo "Number of threads: " $i
	echo "RingSTM: " $ringsw_time
	echo "NOrec: " $NOrec_time
	echo "TL2: " $tl2_time
	echo "HTM: " $htm_time
	echo "Part: " $part_time
	echo

	echo $i $ringsw_time $NOrec_time $tl2_time $htm_time $part_time >> "yada.csv"
	
done

#-----------------------------------------------------------------------------------------------------------------------

echo "Genome"
echo
:>"genome.csv"
for i in 1 2 4 8
do

	ringsw_time=0
	NOrec_time=0
	tl2_time=0
	htm_time=0
	part_time=0

for (( j = 1 ; j <= 5; j++ ))
do

	export STM_CONFIG=RingSW
	temp=$(echo "$(/home/mohamedin/Downloads/orig_rstm_2/rstm_build/stamp-0.9.10/genome/genomeSTM64 -g16384 -s64 -n16777216 -t$i)" | egrep -e 'Time[ ]*=' | egrep -e [1234567890.]* -o)
	ringsw_time=$(echo $ringsw_time $temp | awk ' { printf "%f\n", $1 + $2 } ') 

	export STM_CONFIG=NOrecRH
	temp=$(echo "$(/home/mohamedin/Downloads/rstm_rh/rstm_build/stamp-0.9.10/genome/genomeSTM64 -g16384 -s64 -n16777216 -t$i)" | egrep -e 'Time[ ]*=' | egrep -e [1234567890.]* -o)
	NOrec_time=$(echo $NOrec_time $temp | awk ' { printf "%f\n", $1 + $2 } ') 

	export STM_CONFIG=LLT
	temp=$(echo "$(/home/mohamedin/Downloads/orig_rstm_2/rstm_build/stamp-0.9.10/genome/genomeSTM64 -g16384 -s64 -n16777216 -t$i)" | egrep -e 'Time[ ]*=' | egrep -e [1234567890.]* -o)
	tl2_time=$(echo $tl2_time $temp | awk ' { printf "%f\n", $1 + $2 } ') 

	temp=$(echo "$(/home/mohamedin/stamp-tsx/genome/genome -g16384 -s64 -n16777216 -t$i)" | egrep -e 'Time[ ]*=' | egrep -e [1234567890.]* -o)
	htm_time=$(echo $htm_time $temp | awk ' { printf "%f\n", $1 + $2 } ') 


	temp=$(echo "$(/home/mohamedin/git/ft-stm/STAMP/genome/genome -g16384 -s64 -n16777216 -t$i)" | egrep -e 'Time[ ]*=' | egrep -e [1234567890.]* -o)
	part_time=$(echo $part_time $temp | awk ' { printf "%f\n", $1 + $2 } ') 

done
	
	ringsw_time=$(echo $ringsw_time 5.0 | awk ' { printf "%f\n", $1 / $2 } ') 
	NOrec_time=$(echo $NOrec_time 5.0 | awk ' { printf "%f\n", $1 / $2 } ')
	tl2_time=$(echo $tl2_time 5.0 | awk ' { printf "%f\n", $1 / $2 } ')
	htm_time=$(echo $htm_time 5.0 | awk ' { printf "%f\n", $1 / $2 } ')
	part_time=$(echo $part_time 5.0 | awk ' { printf "%f\n", $1 / $2 } ')	

	echo "Number of threads: " $i
	echo "RingSTM: " $ringsw_time
	echo "NOrec: " $NOrec_time
	echo "TL2: " $tl2_time
	echo "HTM: " $htm_time
	echo "Part: " $part_time
	echo

	echo $i $ringsw_time $NOrec_time $tl2_time $htm_time $part_time >> "genome.csv"
	
done

#-----------------------------------------------------------------------------------------------------------------------

echo "Bayes"
echo
:>"bayes.csv"
for i in 1 2 4 8
do

	ringsw_time=0
	NOrec_time=0
	tl2_time=0
	htm_time=0
	part_time=0

for (( j = 1 ; j <= 5; j++ ))
do

	export STM_CONFIG=RingSW
	temp=$(echo "$(/home/mohamedin/Downloads/orig_rstm_2/rstm_build/stamp-0.9.10/bayes/bayesSTM64 -v32 -r4096 -n10 -p40 -i2 -e8 -s1 -t$i)" | egrep -e 'Learn time[ ]*=' | egrep -e [1234567890.]* -o)
	ringsw_time=$(echo $ringsw_time $temp | awk ' { printf "%f\n", $1 + $2 } ') 

	export STM_CONFIG=NOrecRH
	temp=$(echo "$(/home/mohamedin/Downloads/rstm_rh/rstm_build/stamp-0.9.10/bayes/bayesSTM64 -v32 -r4096 -n10 -p40 -i2 -e8 -s1 -t$i)" | egrep -e 'Learn time[ ]*=' | egrep -e [1234567890.]* -o)
	NOrec_time=$(echo $NOrec_time $temp | awk ' { printf "%f\n", $1 + $2 } ') 

	export STM_CONFIG=LLT
	temp=$(echo "$(/home/mohamedin/Downloads/orig_rstm_2/rstm_build/stamp-0.9.10/bayes/bayesSTM64 -v32 -r4096 -n10 -p40 -i2 -e8 -s1 -t$i)" | egrep -e 'Learn time[ ]*=' | egrep -e [1234567890.]* -o)
	tl2_time=$(echo $tl2_time $temp | awk ' { printf "%f\n", $1 + $2 } ') 

	temp=$(echo "$(/home/mohamedin/stamp-tsx/bayes/bayes -v32 -r4096 -n10 -p40 -i2 -e8 -s1 -t$i)" | egrep -e 'Learn time[ ]*=' | egrep -e [1234567890.]* -o)
	htm_time=$(echo $htm_time $temp | awk ' { printf "%f\n", $1 + $2 } ') 


	temp=$(echo "$(/home/mohamedin/git/ft-stm/STAMP/bayes/bayes -v32 -r4096 -n10 -p40 -i2 -e8 -s1 -t$i)" | egrep -e 'Learn time[ ]*=' | egrep -e [1234567890.]* -o)
	part_time=$(echo $part_time $temp | awk ' { printf "%f\n", $1 + $2 } ') 
	
done
	
	ringsw_time=$(echo $ringsw_time 5.0 | awk ' { printf "%f\n", $1 / $2 } ') 
	NOrec_time=$(echo $NOrec_time 5.0 | awk ' { printf "%f\n", $1 / $2 } ')
	tl2_time=$(echo $tl2_time 5.0 | awk ' { printf "%f\n", $1 / $2 } ')
	htm_time=$(echo $htm_time 5.0 | awk ' { printf "%f\n", $1 / $2 } ')
	part_time=$(echo $part_time 5.0 | awk ' { printf "%f\n", $1 / $2 } ')	

	echo "Number of threads: " $i
	echo "RingSTM: " $ringsw_time
	echo "NOrec: " $NOrec_time
	echo "TL2: " $tl2_time
	echo "HTM: " $htm_time
	echo "Part: " $part_time
	echo

	echo $i $ringsw_time $NOrec_time $tl2_time $htm_time $part_time >> "bayes.csv"
	
done

#-----------------------------------------------------------------------------------------------------------------------

echo "Intruder"
echo
:>"intruder.csv"
for i in 1 2 4 8
do

	ringsw_time=0
	NOrec_time=0
	tl2_time=0
	htm_time=0
	part_time=0

for (( j = 1 ; j <= 5; j++ ))
do

	export STM_CONFIG=RingSW
	temp=$(echo "$(/home/mohamedin/Downloads/orig_rstm_2/rstm_build/stamp-0.9.10/intruder/intruderSTM64 -a10 -l128 -n262144 -s1 -t$i)" | egrep -e 'Elapsed time[ ]*= [^ ]*' -o | egrep -e [1234567890.]* -o)
	ringsw_time=$(echo $ringsw_time $temp | awk ' { printf "%f\n", $1 + $2 } ') 

	export STM_CONFIG=NOrecRH
	temp=$(echo "$(/home/mohamedin/Downloads/rstm_rh/rstm_build/stamp-0.9.10/intruder/intruderSTM64 -a10 -l128 -n262144 -s1 -t$i)" | egrep -e 'Elapsed time[ ]*= [^ ]*' -o | egrep -e [1234567890.]* -o)
	NOrec_time=$(echo $NOrec_time $temp | awk ' { printf "%f\n", $1 + $2 } ') 

	export STM_CONFIG=LLT
	temp=$(echo "$(/home/mohamedin/Downloads/orig_rstm_2/rstm_build/stamp-0.9.10/intruder/intruderSTM64 -a10 -l128 -n262144 -s1 -t$i)" | egrep -e 'Elapsed time[ ]*= [^ ]*' -o | egrep -e [1234567890.]* -o)
	tl2_time=$(echo $tl2_time $temp | awk ' { printf "%f\n", $1 + $2 } ') 

	temp=$(echo "$(/home/mohamedin/stamp-tsx/intruder/intruder -a10 -l128 -n262144 -s1 -t$i)" | egrep -e 'Elapsed time[ ]*= [^ ]*' -o | egrep -e [1234567890.]* -o)
	htm_time=$(echo $htm_time $temp | awk ' { printf "%f\n", $1 + $2 } ') 


	temp=$(echo "$(/home/mohamedin/git/ft-stm/STAMP/intruder/intruder -a10 -l128 -n262144 -s1 -t$i)" | egrep -e 'Elapsed time[ ]*= [^ ]*' -o | egrep -e [1234567890.]* -o)
	part_time=$(echo $part_time $temp | awk ' { printf "%f\n", $1 + $2 } ') 

done
	
	ringsw_time=$(echo $ringsw_time 5.0 | awk ' { printf "%f\n", $1 / $2 } ') 
	NOrec_time=$(echo $NOrec_time 5.0 | awk ' { printf "%f\n", $1 / $2 } ')
	tl2_time=$(echo $tl2_time 5.0 | awk ' { printf "%f\n", $1 / $2 } ')
	htm_time=$(echo $htm_time 5.0 | awk ' { printf "%f\n", $1 / $2 } ')
	part_time=$(echo $part_time 5.0 | awk ' { printf "%f\n", $1 / $2 } ')	

	echo "Number of threads: " $i
	echo "RingSTM: " $ringsw_time
	echo "NOrec: " $NOrec_time
	echo "TL2: " $tl2_time
	echo "HTM: " $htm_time
	echo "Part: " $part_time
	echo

	echo $i $ringsw_time $NOrec_time $tl2_time $htm_time $part_time >> "intruder.csv"
	
done
