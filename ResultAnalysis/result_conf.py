import sys

RQ_number = sys.argv[1]
epoch_number = sys.argv[2]
result_dir = sys.argv[3]
deep_data_dir = sys.argv[4]

loss_function = 'softmax'
subs = ['Chart','Lang','Math','Time','Mockito','Closure']
vers = [26, 65, 106, 27, 38, 133]