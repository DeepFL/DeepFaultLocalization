  #! /bin/bash
for i in $(seq 11 $1)
do
	./multiple_run.sh 7 8 40 $i
	echo "$i done---------------------------- "
done


