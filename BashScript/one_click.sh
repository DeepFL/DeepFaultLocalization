  #! /bin/bash
start=$1
end=$2
gpu_count=$3
name=$4
iter=$5
result=$6
for i in $(seq $start $((end-1)))
do
	python main.py . ./result/$result $name $i fc DeepFL softmax $iter 1 $gpu_count;
	#echo "$name $i"
done
