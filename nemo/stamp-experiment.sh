echo "Kmeans Hi"
echo
:>"kmeansHi.csv"
for i in 1 4 8 16 24 32 40 48 56 64 72
do

	htm_time=0
	obj_time=0
	
	
for (( j = 1 ; j <= 5; j++ ))
do

	temp=$(echo "$(/home/users/mohamed2/nemo/kmeans/kmeans -m15 -n15 -t0.00001 -i /home/users/mohamed2/nemo/kmeans/inputs/random-n65536-d32-c16.txt -p$i)" | egrep -e 'Time: [^ ]*' -o | cut -d ' ' -f 2)
	htm_time=$(echo $htm_time $temp | awk ' { printf "%f\n", $1 + $2 } ') 

	temp=$(echo "$(/home/users/mohamed2/nemo/kmeans/kmeansObj -m15 -n15 -t0.00001 -i /home/users/mohamed2/nemo/kmeans/inputs/random-n65536-d32-c16.txt -p$i)" | egrep -e 'Time: [^ ]*' -o | cut -d ' ' -f 2)
	obj_time=$(echo $obj_time $temp | awk ' { printf "%f\n", $1 + $2 } ') 

done
	
	htm_time=$(echo $htm_time 5.0 | awk ' { printf "%f\n", $1 / $2 } ')
	obj_time=$(echo $obj_time 5.0 | awk ' { printf "%f\n", $1 / $2 } ')
	
	echo "Number of threads: " $i
	echo "HTM: " $htm_time
	echo

	echo $i $htm_time $obj_time >> "kmeansHi.csv"
	
done

#-----------------------------------------------------------------------------------------------------------------------

echo "Kmeans Low"
echo
:>"kmeansLow.csv"
for i in 1 4 8 16 24 32 40 48 56 64 72
do

	part_time=0
	obj_time=0

for (( j = 1 ; j <= 5; j++ ))
do

	temp=$(echo "$(/home/users/mohamed2/nemo/kmeans/kmeans -m40 -n40 -t0.00001  -i /home/users/mohamed2/nemo/kmeans/inputs/random-n65536-d32-c16.txt -p$i)" | egrep -e 'Time: [^ ]*' -o | cut -d ' ' -f 2)
	part_time=$(echo $part_time $temp | awk ' { printf "%f\n", $1 + $2 } ') 

	temp=$(echo "$(/home/users/mohamed2/nemo/kmeans/kmeansObj -m40 -n40 -t0.00001  -i /home/users/mohamed2/nemo/kmeans/inputs/random-n65536-d32-c16.txt -p$i)" | egrep -e 'Time: [^ ]*' -o | cut -d ' ' -f 2)
	obj_time=$(echo $obj_time $temp | awk ' { printf "%f\n", $1 + $2 } ') 

done
	
	part_time=$(echo $part_time 5.0 | awk ' { printf "%f\n", $1 / $2 } ')
	obj_time=$(echo $obj_time 5.0 | awk ' { printf "%f\n", $1 / $2 } ')

	echo "Number of threads: " $i
	echo "Part: " $part_time
	echo

	echo $i $part_time $obj_time >> "kmeansLow.csv"
	
done

#-----------------------------------------------------------------------------------------------------------------------


echo "ssca2"
echo
:>"ssca2.csv"
for i in 1 4 8 16 24 32 40 48 56 64 72 
do

	part_time=0
	obj_time=0

for (( j = 1 ; j <= 5; j++ ))
do

	temp=$(echo "$(/home/users/mohamed2/nemo/ssca2/ssca2 -s20 -i1.0 -u1.0 -l3 -p3 -t$i)"  | egrep -e 'Time taken for all is' | egrep -e [1234567890.]* -o)
	part_time=$(echo $part_time $temp | awk ' { printf "%f\n", $1 + $2 } ') 

	temp=$(echo "$(/home/users/mohamed2/nemo/ssca2/ssca2Obj -s20 -i1.0 -u1.0 -l3 -p3 -t$i)"  | egrep -e 'Time taken for all is' | egrep -e [1234567890.]* -o)
	obj_time=$(echo $obj_time $temp | awk ' { printf "%f\n", $1 + $2 } ') 

done

	part_time=$(echo $part_time 5.0 | awk ' { printf "%f\n", $1 / $2 } ')
	obj_time=$(echo $obj_time 5.0 | awk ' { printf "%f\n", $1 / $2 } ')

	echo "Number of threads: " $i
	echo "Part: " $part_time
	echo

	echo $i $part_time $obj_time >> "ssca2.csv"
	
done

#-----------------------------------------------------------------------------------------------------------------------

echo "labyrinth"
echo
:>"labyrinth.csv"
for i in 1 4 8 16 24 32 40 48 56 64 72 
do

	part_time=0
	obj_time=0

for (( j = 1 ; j <= 5; j++ ))
do	


	temp=$(echo "$(/home/users/mohamed2/nemo/labyrinth/labyrinth -i /home/users/mohamed2/nemo/labyrinth/inputs/random-x512-y512-z7-n512.txt -t$i)"  | egrep -e 'Elapsed time    = [^ ]*' -o | cut -d '=' -f 2 | cut -d ' ' -f 2)
	part_time=$(echo $part_time $temp | awk ' { printf "%f\n", $1 + $2 } ') 

	temp=$(echo "$(/home/users/mohamed2/nemo/labyrinth/labyrinthObj -i /home/users/mohamed2/nemo/labyrinth/inputs/random-x512-y512-z7-n512.txt -t$i)"  | egrep -e 'Elapsed time    = [^ ]*' -o | cut -d '=' -f 2 | cut -d ' ' -f 2)
	obj_time=$(echo $obj_time $temp | awk ' { printf "%f\n", $1 + $2 } ') 

done
	
	part_time=$(echo $part_time 5.0 | awk ' { printf "%f\n", $1 / $2 } ')
	obj_time=$(echo $obj_time 5.0 | awk ' { printf "%f\n", $1 / $2 } ')
	
	echo "Number of threads: " $i
	echo "Part: " $part_time
	echo

	echo $i $part_time $obj_time >> "labyrinth.csv"
	
done

#-----------------------------------------------------------------------------------------------------------------------

echo "Vacation High"
echo
:>"vacationHi.csv"
for i in 1 4 8 16 24 32 40 48 56 64 72
do

	part_time=0
	obj_time=0

for (( j = 1 ; j <= 5; j++ ))
do

	temp=$(echo "$(/home/users/mohamed2/nemo/vacation/vacation -n4 -q60 -u90 -r1048576 -t4194304 -c$i)" | egrep -e 'Time = [^ ]*' -o | cut -d ' ' -f 3)
	part_time=$(echo $part_time $temp | awk ' { printf "%f\n", $1 + $2 } ') 

	temp=$(echo "$(/home/users/mohamed2/nemo/vacation/vacationObj -n4 -q60 -u90 -r1048576 -t4194304 -c$i)" | egrep -e 'Time = [^ ]*' -o | cut -d ' ' -f 3)
	obj_time=$(echo $obj_time $temp | awk ' { printf "%f\n", $1 + $2 } ') 

done
	
	part_time=$(echo $part_time 5.0 | awk ' { printf "%f\n", $1 / $2 } ')	
	obj_time=$(echo $obj_time 5.0 | awk ' { printf "%f\n", $1 / $2 } ')
	
	echo "Number of threads: " $i
	echo "Part: " $part_time
	echo

	echo $i $part_time $obj_time  >> "vacationHi.csv"
	
done

#-----------------------------------------------------------------------------------------------------------------------

echo "Vacation Low"
echo
:>"vacationLow.csv"
for i in 1 4 8 16 24 32 40 48 56 64 72
do

	part_time=0
	obj_time=0

for (( j = 1 ; j <= 5; j++ ))
do

	temp=$(echo "$(/home/users/mohamed2/nemo/vacation/vacation -n2 -q90 -u98 -r1048576 -t4194304 -c$i)" | egrep -e 'Time = [^ ]*' -o | cut -d ' ' -f 3)
	part_time=$(echo $part_time $temp | awk ' { printf "%f\n", $1 + $2 } ') 

	temp=$(echo "$(/home/users/mohamed2/nemo/vacation/vacationObj -n2 -q90 -u98 -r1048576 -t4194304 -c$i)" | egrep -e 'Time = [^ ]*' -o | cut -d ' ' -f 3)
	obj_time=$(echo $obj_time $temp | awk ' { printf "%f\n", $1 + $2 } ') 

done
	
	part_time=$(echo $part_time 5.0 | awk ' { printf "%f\n", $1 / $2 } ')	
	obj_time=$(echo $obj_time 5.0 | awk ' { printf "%f\n", $1 / $2 } ')
	
	echo "Number of threads: " $i
	echo "Part: " $part_time
	echo

	echo $i $part_time $obj_time >> "vacationLow.csv"
	
done

#-----------------------------------------------------------------------------------------------------------------------

echo "Yada"
echo
:>"yada.csv"
for i in 1 4 8 16 24 32 40 48 56 64 72
do

	part_time=0
	obj_time=0

for (( j = 1 ; j <= 5; j++ ))
do

	temp=$(echo "$(/home/users/mohamed2/nemo/yada/yada -a15 -i /home/users/mohamed2/nemo/yada/inputs/ttimeu1000000.2 -t$i)" | egrep -e 'Elapsed time[ ]*= [^ ]*' -o | egrep -e [1234567890.]* -o)
	part_time=$(echo $part_time $temp | awk ' { printf "%f\n", $1 + $2 } ') 

	temp=$(echo "$(/home/users/mohamed2/nemo/yada/yadaObj -a15 -i /home/users/mohamed2/nemo/yada/inputs/ttimeu1000000.2 -t$i)" | egrep -e 'Elapsed time[ ]*= [^ ]*' -o | egrep -e [1234567890.]* -o)
	obj_time=$(echo $obj_time $temp | awk ' { printf "%f\n", $1 + $2 } ') 

done
	
	part_time=$(echo $part_time 5.0 | awk ' { printf "%f\n", $1 / $2 } ')	
	obj_time=$(echo $obj_time 5.0 | awk ' { printf "%f\n", $1 / $2 } ')

	echo "Number of threads: " $i
	echo "Part: " $part_time
	echo

	echo $i $part_time $obj_time >> "yada.csv"
	
done

#-----------------------------------------------------------------------------------------------------------------------

echo "Genome"
echo
:>"genome.csv"
for i in 1 4 8 16 24 32 40 48 56 64 72
do

	part_time=0
	obj_time=0

for (( j = 1 ; j <= 5; j++ ))
do
	temp=$(echo "$(/home/users/mohamed2/nemo/genome/genome -g16384 -s64 -n16777216 -t$i)" | egrep -e 'Time[ ]*=' | egrep -e [1234567890.]* -o)
	part_time=$(echo $part_time $temp | awk ' { printf "%f\n", $1 + $2 } ') 

	temp=$(echo "$(/home/users/mohamed2/nemo/genome/genomeObj -g16384 -s64 -n16777216 -t$i)" | egrep -e 'Time[ ]*=' | egrep -e [1234567890.]* -o)
	obj_time=$(echo $obj_time $temp | awk ' { printf "%f\n", $1 + $2 } ') 

done
	
	part_time=$(echo $part_time 5.0 | awk ' { printf "%f\n", $1 / $2 } ')	
	obj_time=$(echo $obj_time 5.0 | awk ' { printf "%f\n", $1 / $2 } ')
	
	echo "Number of threads: " $i
	echo "Part: " $part_time
	echo

	echo $i $part_time $obj_time >> "genome.csv"
	
done

#-----------------------------------------------------------------------------------------------------------------------

echo "Intruder"
echo
:>"intruder.csv"
for i in 1 4 8 16 24 32 40 48 56 64 72
do

	part_time=0
	obj_time=0

for (( j = 1 ; j <= 5; j++ ))
do


	temp=$(echo "$(/home/users/mohamed2/nemo/intruder/intruder -a10 -l128 -n262144 -s1 -t$i)" | egrep -e 'Elapsed time[ ]*= [^ ]*' -o | egrep -e [1234567890.]* -o)
	part_time=$(echo $part_time $temp | awk ' { printf "%f\n", $1 + $2 } ') 

	temp=$(echo "$(/home/users/mohamed2/nemo/intruder/intruderObj -a10 -l128 -n262144 -s1 -t$i)" | egrep -e 'Elapsed time[ ]*= [^ ]*' -o | egrep -e [1234567890.]* -o)
	obj_time=$(echo $obj_time $temp | awk ' { printf "%f\n", $1 + $2 } ') 

done
	
	part_time=$(echo $part_time 5.0 | awk ' { printf "%f\n", $1 / $2 } ')	
	obj_time=$(echo $obj_time 5.0 | awk ' { printf "%f\n", $1 / $2 } ')

	echo "Number of threads: " $i
	echo "Part: " $part_time
	echo

	echo $i $part_time $obj_time >> "intruder.csv"
	
done