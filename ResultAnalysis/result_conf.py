import sys

RQ = sys.argv[1]
epoch_number = sys.argv[1]
result_dir = sys.argv[2]
deep_data_dir = sys.argv[3]




loss_function = 'softmax'
subs = ['Chart','Lang','Math','Time','Mockito','Closure']
vers = [26, 65, 106, 27, 38, 133]
libsvm_results = ['Ochiai','MeOchiai','Multric','Fluccs','Trapt']