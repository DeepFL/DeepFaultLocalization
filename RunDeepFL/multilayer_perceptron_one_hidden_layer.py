

from __future__ import print_function

import input
import tensorflow as tf
import time
from config import *
import utils as ut


'''
Define MLP model
@param x: input tensor of model
@param weights: weights of model in a dictionary format
@param biases: biases of model in a dictionary format
@param keep_prob: keep probability in drop-out layer, which is defined in config.py
@return tensor of prediction result
'''
def multilayer_perceptron(x, weights, biases, keep_prob):
    # Hidden layer with sigmoid activation
    layer_1 = tf.add(tf.matmul(x, weights['h1']), biases['b1'])
    layer_1 = tf.nn.sigmoid(layer_1)
    drop_out = tf.nn.dropout(layer_1, keep_prob)
    out_layer = tf.matmul(drop_out, weights['out']) + biases['out']
    return out_layer
    
'''
Main function for executing the model
@param trainFile: .csv filename of training features
@param trainLabelFile: .csv filename of training labels
@param testFile: .csv filename of test features
@param testLabelFile: .csv filename of test labels
@param groupFile: group filename
@param suspFile: output file name storing the prediction results of model, typically the results name will be suspFile+epoch_num
@param loss: the loss function configurations controlled in command
@param featureNum: number of input features
@param nodeNum: hidden node number per layer 
@return N/A
'''
def run(trainFile, trainLabelFile, testFile,testLabelFile, groupFile, suspFile,loss, featureNum, nodeNum):
    tf.reset_default_graph()
    # Network Parameters
    n_classes = 2 #  total output classes (0 or 1)
    n_input = featureNum # total number of input features
    n_hidden_1 = nodeNum # 1st layer number of nodes                                                                       
    
    # tf Graph input
    x = tf.placeholder("float", [None, n_input])
    y = tf.placeholder("float", [None, n_classes])
    g = tf.placeholder(tf.int32, [None, 1])
  
    # dropout parameter
    keep_prob = tf.placeholder(tf.float32)

    # Store layers weight & bias
    weights = {
        'h1': tf.Variable(tf.random_normal([n_input, n_hidden_1])),
        'out': tf.Variable(tf.random_normal([n_hidden_1, n_classes]))
    }
    biases = {
        'b1': tf.Variable(tf.random_normal([n_hidden_1])),
        'out': tf.Variable(tf.random_normal([n_classes]))
    }

    # Construct model
    pred = multilayer_perceptron(x, weights, biases, keep_prob)
   
    datasets=input.read_data_sets(trainFile, trainLabelFile, testFile, testLabelFile, groupFile)


    # Define loss and optimizer                                                                                                                                                                               
    variables  = tf.trainable_variables()
    regularizer = (tf.nn.l2_loss(weights['h1'])+tf.nn.l2_loss(weights['out'])) * L2_value   # l2 regularization               
    cost = ut.loss_func(pred, y, loss, datasets,g)
    optimizer = tf.train.AdamOptimizer(learning_rate=learning_rate).minimize(cost+regularizer)

    # Initializing the variables
    init = tf.global_variables_initializer()

    # Launch the graph
    gpu_options = tf.GPUOptions(per_process_gpu_memory_fraction=0.1)
    with tf.Session(config=tf.ConfigProto(gpu_options=gpu_options)) as sess:
        sess.run(init)

        # Training cycle
        for epoch in range(training_epochs):
            avg_cost = 0.
            total_batch = int(datasets.train.num_instances/batch_size)
            # Loop over all batches
            for i in range(total_batch):
                batch_x, batch_y ,batch_g= datasets.train.next_batch(batch_size)
                # Run optimization op (backprop) and cost op (to get loss value)
                _, c = sess.run([optimizer, cost], feed_dict={x: batch_x,
                                                              y: batch_y, g: batch_g, keep_prob: dropout_rate})
                # Compute average loss
                avg_cost += c / total_batch
            # Display logs per epoch step
            if epoch % display_step == 0:
                print("Epoch:", '%04d' % (epoch+1), "cost=", \
                    "{:.9f}".format(avg_cost))
            if epoch % dump_step ==(dump_step-1):
                #Write Result
                res=sess.run(tf.nn.softmax(pred),feed_dict={x: datasets.test.instances, y: datasets.test.labels, keep_prob: 1.0})
                with open(suspFile+'-'+str(epoch+1),'w') as f:
                    for susp in res[:,0]:
                        f.write(str(susp)+'\n')

        print("Optimization Finished!")
