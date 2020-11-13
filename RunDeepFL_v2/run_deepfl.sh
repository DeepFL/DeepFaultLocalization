#!/bin/bash
# subs=(Chart Lang Time Math Mockito Closure)
# vers=(26 65 27 106 38 133)
# subs=(Lang Time Math Mockito Closure)
# vers=(65 27 106 38 133)
subs=(Closure)
vers=(133)
#First iteration for executing all kinds of models with different dimensions of features
#Related result in paper: Table 4, Table 5, Table 6

for idx in {0..0}
do
	sub=${subs[$idx]}
	# for model in {attention}
	# do
	ver_for_sub=${vers[$idx]}
	echo $model
	for v in `seq 1 $ver_for_sub`
	do
		echo $sub
		echo $v
		srun --gres=gpu:1 -p gpu_2h python3 main.py .. ../Results $sub $v attention DeepFL softmax 60 1;
	done
	# done
done

#Second iteration for executing models using epariwise as loss function
#Related result in paper: Figure 8
# for idx in {0..5}
# do
# 	sub=${subs[$idx]}
# 	ver_for_sub=${vers[$idx]}
# 	for v in `seq 1 $ver_for_sub`
# 		do
# 			echo $sub
# 			echo $v
# 			python main.py .. ../Results $sub $v dfl2 DeepFL epairwise 60 1;
# 		done
# done

#Third iteration for using CrossDeepFL as tech
#Related result in paper: Figure 9
# for idx in {0..5}
# do
# 	sub=${subs[$idx]}
# 	ver_for_sub=${vers[$idx]}
# 	for v in `seq 1 $ver_for_sub`
# 		do
# 			echo $sub
# 			echo $v
# 			python main.py .. ../Results $sub $v dfl2 CrossDeepFL softmax 60 1;
# 		done
# done

#Fourth iteration for using CrossValidation as tech
#Related result in paper: Figure 9
# for idx in {1..10}
# do
# 	echo $idx
# 	python main.py .. ../Results 10fold $idx dfl2 CrossValidation softmax 60 1;
# done