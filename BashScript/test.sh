  #! /bin/bash
iter=$1
gpu=$2
data=$3
if [ $data = "all" ]
then   
  for i in {1..26}
  do
  python main.py . ./result/ Chart $i fc DeepFL softmax $iter 10 $gpu;
  done
  for i in {1..133}
  do
  python main.py . ./result/ Closure $i fc DeepFL softmax $iter 10 $gpu;
  done
  for i in {1..65}
  do
  python main.py . ./result/ Lang $i fc DeepFL softmax $iter 10 $gpu;
  done
  for i in {1..106}
  do
  python main.py . ./result/ Math $i fc DeepFL softmax $iter 10 $gpu;
  done
  for i in {1..38}
  do
  python main.py . ./result/ Mockito $i fc DeepFL softmax $iter 10 $gpu;
  done
  for i in {1..27}
  do
  python main.py . ./result/ Time $i fc DeepFL softmax $iter 10 $gpu;
  done
elif [ $data = "Chart" ]
then
  for i in {1..26}
  do
  python main.py . ./result/ Chart $i fc DeepFL softmax $iter 10 $gpu;
  done
elif [ $data = "Closure" ]
then
  for i in {1..133}
  do
  python main.py . ./result/ Closure $i fc DeepFL softmax $iter 10 $gpu;
  done
elif [ $data = "Lang" ]
then
  for i in {1..65}
  do
  python main.py . ./result/ Lang $i fc DeepFL softmax $iter 10 $gpu;
  done
elif [ $data = "Math" ]
then
  for i in {1..106}
  do
  python main.py . ./result/ Math $i fc DeepFL softmax $iter 10 $gpu;
  done
elif [ $data = "Mockito" ]
then
  for i in {1..38}
  do
  python main.py . ./result/ Mockito $i fc DeepFL softmax $iter 10 $gpu;
  done
elif [ $data = "Time" ]
then
  for i in {1..27}
  do
  python main.py . ./result/ Time $i fc DeepFL softmax $iter 10 $gpu;
  done
fi