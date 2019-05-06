  #! /bin/bash
gpu_num=$1
work_per_gpu=$2
iter=$3
result=$4
./quick_script.sh $gpu_num Lang $work_per_gpu 65 $iter $result
./quick_script.sh $gpu_num Time $work_per_gpu 27 $iter $result
./quick_script.sh $gpu_num Math $work_per_gpu 106 $iter $result
./quick_script.sh $gpu_num Chart $work_per_gpu 26 $iter $result
./quick_script.sh $gpu_num Mockito $work_per_gpu 38 $iter $result
./quick_script.sh $gpu_num Closure $work_per_gpu 133 $iter $result