  #! /bin/bash
gpu_num=$1
name=$2
work_per_gpu=$3
total_sub=$4
iter=$5
result=$6
loop=$(($work_per_gpu*$gpu_num))
sub_per_loop=$(($total_sub/$loop))
remainder=$(($total_sub%$loop))
start=1
end=1
loop=$(($loop-1))
#echo "$loop $sub_per_loop $remainder"
for i in $(seq 0 $loop)
do
  if [ $i -lt $remainder ]
  then
    end=$(($start + $sub_per_loop + 1))
  else
    end=$(($start + $sub_per_loop))
  fi
  gpu_count=$(($i / $work_per_gpu + 1))
  echo "$gpu_count $end $start"
  ./one_click.sh $start $end $gpu_count $name $iter $result &
  BACK_PID[$i]=$!
  start=$end
done
for i in $(seq 0 $loop)
do 
  wait ${BACK_PID[$i]}
done
