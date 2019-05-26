

from __future__ import print_function

import input
import tensorflow as tf
import time
from config import *
import utils as ut


# Create model
def multilayer_perceptron(x, weights, biases, keep_prob):
    # Hidden layer with sigmoid activation
    layer_1 = tf.add(tf.matmul(x, weights['h1']), biases['b1'])
    layer_1 = tf.nn.sigmoid(layer_1)
    drop_out = tf.nn.dropout(layer_1, keep_prob)
    out_layer = tf.matmul(drop_out, weights['out']) + biases['out']
    return out_layer

def mutation_spec_similar_first(spec, m1,m2,m3,m4,complexity,similarity, keep_prob,is_training):
    model_size_times = 2
    with tf.variable_scope('seperate_mut',reuse=False):
        with tf.variable_scope('mut1',reuse=False):
            #mutation = tf.layers.batch_normalization(mutation, training=is_training)
            mut_1 = single_fc_layer(m1,35,35*model_size_times, keep_prob,is_training)
        with tf.variable_scope('mut2',reuse=False):
            #mutation = tf.layers.batch_normalization(mutation, training=is_training)
            mut_2 = single_fc_layer(m2,35,35*model_size_times, keep_prob,is_training)
        with tf.variable_scope('mut3',reuse=False):
            #mutation = tf.layers.batch_normalization(mutation, training=is_training)
            mut_3 = single_fc_layer(m3,35,35*model_size_times, keep_prob,is_training)
        with tf.variable_scope('mut4',reuse=False):
            #mutation = tf.layers.batch_normalization(mutation, training=is_training)
            mut_4 = single_fc_layer(m4,35,35*model_size_times, keep_prob,is_training)
        mut_concat = tf.concat([mut_1,mut_2,mut_3,mut_4],1)
        with tf.variable_scope('mut_concat',reuse=False):
            mut_concat = single_fc_layer(mut_concat,35*4*model_size_times,35*model_size_times, keep_prob,is_training)

    with tf.variable_scope('seperate_spec',reuse=False):
        with tf.variable_scope('spec',reuse=False):
            #spec = tf.layers.batch_normalization(spec, training=is_training)
            spec_1 = single_fc_layer(spec,34,34*model_size_times, keep_prob,is_training)
        spec_concat = tf.concat([spec_1,mut_concat],1)
        with tf.variable_scope('fc1',reuse=False):
            spec_concat = single_fc_layer(spec_concat,69*model_size_times,32*model_size_times, keep_prob,is_training)
    with tf.variable_scope('similar_spec',reuse=False):
        with tf.variable_scope('similar',reuse=False):
            #similarity = tf.layers.batch_normalization(similarity, training=is_training)
            similar_1 = single_fc_layer(similarity,15,15*model_size_times, keep_prob,is_training)
        similar_concat = tf.concat([similar_1,spec_concat],1)
        with tf.variable_scope('fc1',reuse=False):
            spec_concat = single_fc_layer(spec_concat,47*model_size_times,32*model_size_times, keep_prob,is_training)
    with tf.variable_scope('fc',reuse=False):
        with tf.variable_scope('complex',reuse=False):
            #complexity = tf.layers.batch_normalization(complexity, training=is_training)
            complex_1 = single_fc_layer(complexity,37,37*model_size_times, keep_prob,is_training)

        fc_1 = tf.concat([complex_1,similar_concat],1)
        with tf.variable_scope('fc1',reuse=False):
            fc_2 = single_fc_layer(fc_1,69*model_size_times,64, keep_prob,is_training)
    final_weight = create_variables("final_weight",[64, 2])
    final_bias = tf.get_variable("final_bias", shape=[2], initializer=tf.zeros_initializer())
    out_layer = tf.add(tf.matmul(fc_2, final_weight), final_bias)
    return out_layer
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
