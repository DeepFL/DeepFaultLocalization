  #! /bin/bash
iter=$1
data=$2
if [ $data = "all" ]
then   
  for i in {1..26}
  do
  python main.py . ./result/ Chart $i dfl1 DeepFL softmax $iter 10;
  done
  for i in {1..133}
  do
  python main.py . ./result/ Closure $i dfl1 DeepFL softmax $iter 10;
  done
  for i in {1..65}
  do
  python main.py . ./result/ Lang $i dfl1 DeepFL softmax $iter 10;
  done
  for i in {1..106}
  do
  python main.py . ./result/ Math $i dfl1 DeepFL softmax $iter 10;
  done
  for i in {1..38}
  do
  python main.py . ./result/ Mockito $i dfl1 DeepFL softmax $iter 10;
  done
  for i in {1..27}
  do
  python main.py . ./result/ Time $i dfl1 DeepFL softmax $iter 10;
  done
elif [ $data = "Chart" ]
then
  for i in {1..26}
  do
  python main.py . ./result/ Chart $i dfl1 DeepFL softmax $iter 10;
  done
elif [ $data = "Closure" ]
then
  for i in {1..133}
  do
  python main.py . ./result/ Closure $i dfl1 DeepFL softmax $iter 10;
  done
elif [ $data = "Lang" ]
then
  for i in {1..65}
  do
  python main.py . ./result/ Lang $i dfl1 DeepFL softmax $iter 10;
  done
elif [ $data = "Math" ]
then
  for i in {1..106}
  do
  python main.py . ./result/ Math $i dfl1 DeepFL softmax $iter 10;
  done
elif [ $data = "Mockito" ]
then
  for i in {1..38}
  do
  python main.py . ./result/ Mockito $i dfl1 DeepFL softmax $iter 10;
  done
elif [ $data = "Time" ]
then
  for i in {1..27}
  do
  python main.py . ./result/ Time $i dfl1 DeepFL softmax $iter 10;
  done
fi