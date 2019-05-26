import tensorflow as tf

'''
loss function for different losses.
@param pred: prediction results of dnn models (before softmax)
@param y: corresponding labels of instances
@param loss: the loss function configurations controlled in command
    0 = weighted softmax
    1 = softmax
    2 = group-based exponential pairwise 
    3 = softmax + group-based exponential pairwise 
    4 = group-based hinge pairwise 
    5 = softmax + group-based hinge pairwise 
    else = optimized pairwise exponential function without grouping
@param datasets: DataSet object 
@param groups: groups introduced in paper
@return loss cost calculated by corresponding loss function
'''
def loss_func(pred, y, loss, datasets, groups=[]):
    if loss == 0: # weighted softmax
        ratio=datasets.train.pos_instance_ratio()
        classes_weights=tf.constant([[1.0-ratio, ratio]])
        weight_per_label = tf.transpose( tf.matmul(y, tf.transpose(classes_weights)) )
        xent = tf.multiply(weight_per_label, tf.nn.softmax_cross_entropy_with_logits_v2(logits=pred, labels=y)) #shape [1, batch_size]              
        cost = tf.reduce_mean(xent)
    elif loss == 1: # softmax
        cost= tf.reduce_mean(tf.nn.softmax_cross_entropy_with_logits_v2(logits=pred, labels=y))
    elif loss == 2 and groups!=[]: # group-based exponential pairwise implementation
        pred_scaled=tf.nn.softmax(pred)
        pred_1=tf.slice(pred_scaled,[0,0],[-1,1])
        y_1=tf.slice(y,[0,0],[-1,1])
        diff=tf.subtract(pred_1,tf.transpose(pred_1))
        grouping=tf.equal(groups, tf.transpose(groups))
        grouping=tf.cast(grouping,tf.float32)
        mask=y_1*tf.transpose(1-y_1)*grouping
        diff_exp=tf.exp(-diff)
        cost=tf.reduce_sum(mask*diff_exp) 
    elif loss == 3 and groups!=[]: # softmax + group-based exponential pairwise implementation
        pred_scaled=tf.nn.softmax(pred)
        pred_1=tf.slice(pred_scaled,[0,0],[-1,1])
        y_1=tf.slice(y,[0,0],[-1,1])
        diff=tf.subtract(pred_1,tf.transpose(pred_1))
        grouping=tf.equal(groups, tf.transpose(groups))
        grouping=tf.cast(grouping,tf.float32)
        mask=y_1*tf.transpose(1-y_1)*grouping
        pair_num=tf.reduce_sum(mask)
        diff_exp=tf.exp(-diff)
        softmax_cost=tf.reduce_mean(tf.nn.softmax_cross_entropy_with_logits_v2(logits=pred, labels=y))
        cost= tf.cond(tf.greater(pair_num,tf.constant(0.0)), lambda: softmax_cost+tf.divide(tf.reduce_sum(mask*diff_exp),pair_num), lambda: softmax_cost)

    elif loss == 4 and groups!=[]: # group-based hinge pairwise implementation
        pred_scaled=tf.nn.softmax(pred)
        pred_1=tf.slice(pred_scaled,[0,0],[-1,1])
        y_1=tf.slice(y,[0,0],[-1,1])
        diff=tf.subtract(pred_1,tf.transpose(pred_1))
        grouping=tf.equal(groups, tf.transpose(groups))
        grouping=tf.cast(grouping,tf.float32)
        mask=y_1*tf.transpose(1-y_1)*grouping
        diff=mask*diff
        cost=tf.reduce_sum(tf.subtract(mask,diff)) 
    elif loss == 5 and groups!=[]: # softmax + group-based hinge pairwise implementation
        pred_scaled=tf.nn.softmax(pred)
        pred_1=tf.slice(pred_scaled,[0,0],[-1,1])
        y_1=tf.slice(y,[0,0],[-1,1])
        diff=tf.subtract(pred_1,tf.transpose(pred_1))
        grouping=tf.equal(groups, tf.transpose(groups))
        grouping=tf.cast(grouping,tf.float32)
        mask=y_1*tf.transpose(1-y_1)*grouping
        pair_num=tf.reduce_sum(mask)
        diff=mask*diff
        softmax_cost=tf.reduce_mean(tf.nn.softmax_cross_entropy_with_logits_v2(logits=pred, labels=y))
        cost= tf.cond(tf.greater(pair_num,tf.constant(0.0)), lambda: softmax_cost+tf.reduce_sum(tf.divide(tf.subtract(mask, diff),pair_num)), lambda: softmax_cost)

    elif False: # optimized pairwise exponential function without grouping, only for reference 
        pred_scaled=tf.nn.softmax(pred)
        pred_1=tf.slice(pred_scaled,[0,0],[-1,1])
        y_1=tf.slice(y,[0,0],[-1,1])
        positive=tf.greater(y_1, 0.0)
        negative=tf.less(y_1, 1.0)
        pred_x=tf.boolean_mask(pred_1,positive)
        pred_x=tf.reshape(pred_x,[-1,1])
        pred_y=tf.boolean_mask(pred_1,negative)
        pred_y=tf.reshape(pred_y,[1,-1])
        diff=tf.subtract(pred_x,pred_y)
        diff_exp=tf.exp(-diff)
        cost=tf.reduce_sum(diff_exp) 
    return cost
