'''
Configuration and parameters can be set in this file
'''


from __future__ import print_function

import tensorflow as tf
from tensorflow.contrib import rnn
import numpy
import input
import time
import sys
                                                                                                                                                                     
techNames = ['DeepFL', 'DeepFL-Spectrum', 'DeepFL-Mutation', 'DeepFL-Metrics', 'DeepFL-Textual', 'CrossDeepFL','CrossValidation']
featureDistr = [[34,35,35,35,35,37,15], [35,35,35,35,37,15], [34,37,15], [34,35,35,35,35,15], [34,35,35,35,35,37], [34,35,35,35,35,37,15],[10]]
featuresize = [226, 192, 86, 189, 211, 226,10]
losses = ['wsoftmax', 'softmax', 'epairwise','epairwiseSoftmax', 'hpairwise','hpairwiseSoftmax'] 

dir = sys.argv[1]      # The directory of dataset
out_dir = sys.argv[2]  # The directory of results
sub = sys.argv[3]      # subject name: ['Time','Chart','Lang','Math','Mockito','Closure']
v = sys.argv[4]  # version number
model = sys.argv[5]  # model name, such as "mlp" "mlp2" "rnn" "birnn"  
tech = sys.argv[6]  # technique name, such as DeepFL, DeepFL-Spectrum, DeepFL-Mutation, DeepFL-Metrics, DeepFL-Textual
loss = sys.argv[7]  # loss function
training_epochs = sys.argv[8]  # training epoch number
dump_step = sys.argv[9] # the number of dumpresult


featureDistribution = featureDistr[techNames.index(tech)]
feature = featuresize[techNames.index(tech)]
                                                                                                               


learning_rate = 0.001
training_epochs = int(training_epochs)
batch_size = 500
display_step = 2
dump_step = int(dump_step)
#dropout_rate = 0.5
dropout_rate = 0.7
L2_value = 0.0001
rnn_hidden = max(featureDistribution)

                                                 
train_file='Train.csv'
train_label_file='TrainLabel.csv'
test_file='Test.csv'
test_label_file='TestLabel.csv'
group_dir='groupfile'
group_file='traidata.txt.group'
susp_file='rank'


