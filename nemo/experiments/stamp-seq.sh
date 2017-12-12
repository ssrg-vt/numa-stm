echo "Kmeans Hi"
echo
:>"kmeansHi.csv"
	htm_time=0
	
for (( j = 1 ; j <= 5; j++ ))
do

	temp=$(echo "$(/home/mohamedin/Downloads/stamp-0.9.10/kmeans/kmeans -m15 -n15 -t0.00001 -i experiments/inputs/random-n65536-d32-c16.txt)" | egrep -e 'Time: [^ ]*' -o | cut -d ' ' -f 2)
	htm_time=$(echo $htm_time $temp | awk ' { printf "%f\n", $1 + $2 } ') 

done
	
	htm_time=$(echo $htm_time 5.0 | awk ' { printf "%f\n", $1 / $2 } ')
	echo "HTM: " $htm_time
	echo

	echo $i $htm_time >> "kmeansHi.csv"

#-----------------------------------------------------------------------------------------------------------------------

echo "Kmeans Low"
echo
:>"kmeansLow.csv"
	htm_time=0

for (( j = 1 ; j <= 5; j++ ))
do

	temp=$(echo "$(/home/mohamedin/Downloads/stamp-0.9.10/kmeans/kmeans -m40 -n40 -t0.00001  -i experiments/inputs/random-n65536-d32-c16.txt)" | egrep -e 'Time: [^ ]*' -o | cut -d ' ' -f 2)
	htm_time=$(echo $htm_time $temp | awk ' { printf "%f\n", $1 + $2 } ') 
done
	
	htm_time=$(echo $htm_time 5.0 | awk ' { printf "%f\n", $1 / $2 } ')

	echo "HTM: " $htm_time
	echo

	echo $i $htm_time >> "kmeansLow.csv"

#-----------------------------------------------------------------------------------------------------------------------


echo "ssca2"
echo
:>"ssca2.csv"
	htm_time=0

for (( j = 1 ; j <= 5; j++ ))
do


	temp=$(echo "$(/home/mohamedin/Downloads/stamp-0.9.10/ssca2/ssca2 -s20 -i1.0 -u1.0 -l3 -p3)"  | egrep -e 'Time taken for all is' | egrep -e [1234567890.]* -o)
	htm_time=$(echo $htm_time $temp | awk ' { printf "%f\n", $1 + $2 } ') 
done

	htm_time=$(echo $htm_time 5.0 | awk ' { printf "%f\n", $1 / $2 } ')
	echo "HTM: " $htm_time
	echo

	echo $i $htm_time >> "ssca2.csv"

#-----------------------------------------------------------------------------------------------------------------------

echo "labyrinth"
echo
:>"labyrinth.csv"
	htm_time=0

for (( j = 1 ; j <= 5; j++ ))
do	
	temp=$(echo "$(/home/mohamedin/Downloads/stamp-0.9.10/labyrinth/labyrinth -i experiments/inputs/random-x512-y512-z7-n512.txt)" | egrep -e 'Elapsed time    = [^ ]*' -o | cut -d '=' -f 2 | cut -d ' ' -f 2)
	htm_time=$(echo $htm_time $temp | awk ' { printf "%f\n", $1 + $2 } ') 
done
	
	htm_time=$(echo $htm_time 5.0 | awk ' { printf "%f\n", $1 / $2 } ')
	echo "HTM: " $htm_time
	echo

	echo $i $htm_time >> "labyrinth.csv"

#-----------------------------------------------------------------------------------------------------------------------

echo "Vacation High"
echo
:>"vacationHi.csv"
	htm_time=0

for (( j = 1 ; j <= 5; j++ ))
do
	temp=$(echo "$(/home/mohamedin/Downloads/stamp-0.9.10/vacation/vacation -n4 -q60 -u90 -r1048576 -t4194304)" | egrep -e 'Time = [^ ]*' -o | cut -d ' ' -f 3)
	htm_time=$(echo $htm_time $temp | awk ' { printf "%f\n", $1 + $2 } ') 
done
	
	htm_time=$(echo $htm_time 5.0 | awk ' { printf "%f\n", $1 / $2 } ')
	echo "HTM: " $htm_time
	echo

	echo $i $htm_time >> "vacationHi.csv"

#-----------------------------------------------------------------------------------------------------------------------

echo "Vacation Low"
echo
:>"vacationLow.csv"
	htm_time=0

for (( j = 1 ; j <= 5; j++ ))
do
	temp=$(echo "$(/home/mohamedin/Downloads/stamp-0.9.10/vacation/vacation -n2 -q90 -u98 -r1048576 -t4194304)" | egrep -e 'Time = [^ ]*' -o | cut -d ' ' -f 3)
	htm_time=$(echo $htm_time $temp | awk ' { printf "%f\n", $1 + $2 } ') 
done
	htm_time=$(echo $htm_time 5.0 | awk ' { printf "%f\n", $1 / $2 } ')
	echo "HTM: " $htm_time
	echo

	echo $i $htm_time >> "vacationLow.csv"

#-----------------------------------------------------------------------------------------------------------------------

echo "Yada"
echo
:>"yada.csv"
	htm_time=0

for (( j = 1 ; j <= 5; j++ ))
do
	temp=$(echo "$(/home/mohamedin/Downloads/stamp-0.9.10/yada/yada -a15 -i experiments/inputs/ttimeu1000000.2 )" | egrep -e 'Elapsed time[ ]*= [^ ]*' -o | egrep -e [1234567890.]* -o)
	htm_time=$(echo $htm_time $temp | awk ' { printf "%f\n", $1 + $2 } ') 
done
	
	htm_time=$(echo $htm_time 5.0 | awk ' { printf "%f\n", $1 / $2 } ')
	echo "HTM: " $htm_time
	echo

	echo $i $htm_time >> "yada.csv"

#-----------------------------------------------------------------------------------------------------------------------

echo "Genome"
echo
:>"genome.csv"
	htm_time=0

for (( j = 1 ; j <= 5; j++ ))
do
	temp=$(echo "$(/home/mohamedin/Downloads/stamp-0.9.10/genome/genome -g16384 -s64 -n16777216 )" | egrep -e 'Time[ ]*=' | egrep -e [1234567890.]* -o)
	htm_time=$(echo $htm_time $temp | awk ' { printf "%f\n", $1 + $2 } ') 
done
	htm_time=$(echo $htm_time 5.0 | awk ' { printf "%f\n", $1 / $2 } ')
	echo "HTM: " $htm_time
	echo

	echo $i $htm_time >> "genome.csv"

#-----------------------------------------------------------------------------------------------------------------------

echo "Bayes"
echo
:>"bayes.csv"
	htm_time=0

for (( j = 1 ; j <= 5; j++ ))
do
	temp=$(echo "$(/home/mohamedin/Downloads/stamp-0.9.10/bayes/bayes -v32 -r4096 -n10 -p40 -i2 -e8 -s1 )" | egrep -e 'Learn time[ ]*=' | egrep -e [1234567890.]* -o)
	htm_time=$(echo $htm_time $temp | awk ' { printf "%f\n", $1 + $2 } ') 
done
	htm_time=$(echo $htm_time 5.0 | awk ' { printf "%f\n", $1 / $2 } ')
	echo "HTM: " $htm_time
	echo

	echo $i $htm_time >> "bayes.csv"

#-----------------------------------------------------------------------------------------------------------------------

echo "Intruder"
echo
:>"intruder.csv"
	htm_time=0

for (( j = 1 ; j <= 5; j++ ))
do
	temp=$(echo "$(/home/mohamedin/Downloads/stamp-0.9.10/intruder/intruder -a10 -l128 -n262144 -s1 )" | egrep -e 'Elapsed time[ ]*= [^ ]*' -o | egrep -e [1234567890.]* -o)
	htm_time=$(echo $htm_time $temp | awk ' { printf "%f\n", $1 + $2 } ') 
done
	htm_time=$(echo $htm_time 5.0 | awk ' { printf "%f\n", $1 / $2 } ')
	echo "HTM: " $htm_time
	echo

	echo $i $htm_time >> "intruder.csv"

