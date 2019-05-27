import os
import numpy
import csv
'''
read data from file
@param filename: name of the .csv file
@return corresponding numpy matrix of csv file
'''
def readFile(filename):
	with open(filename) as f:
		lines=csv.reader(f,delimiter=',')
	 	matrix=[]
		for line in lines:
                        if line[-1] == '':
	                        array=line[:-1]#remove last ',' if any
                        else:
                                array=line
                        matrix.append(array)
		matrix=numpy.asarray(matrix,dtype=numpy.float32)
	#instances=matrix[:,1:]
	#values=numpy.ones((instances.shape[0],1))-matrix[:,0:1]
	#one_hot_labels=numpy.c_[matrix[:,0:1],values]
	return matrix 
'''
read group information, for pairwise loss function
@param filename(string): name of the .csv file
@return corresponding numpy matrix of csv file
'''
def readGroup(filename):
        groups=readFile(filename)
        groups=numpy.asarray(groups,dtype=numpy.int32)
        #groups=numpy.reshape(groups, (-1,1))
        #print(groups.shape)
        results=[]
        for g in range(groups.shape[0]):
                group=numpy.full((1, groups[g][0]), g).ravel()
                results=numpy.concatenate((results,group))
        return results.reshape([-1,1])
        

class DataSet(object):
    def __init__(self,instances,labels, groups=[]):
        self._instances=instances
        self._labels=labels
        self._num_instances=instances.shape[0]
        self._epochs_completed=0
        self._index_in_epoch=0
        self._groups=groups
    @property
    def instances(self):
        return self._instances
    @property
    def labels(self):
        return self._labels
    @property
    def groups(self):
        return self._groups
    @property
    def num_instances(self):
        return self._num_instances
    @property
    def epochs_completed(self):
        return self._epochs_completed
    def pos_instance_ratio(self):
        pos_instances=0.0
        for label in self._labels[:,0]:
                if label==1:
                        pos_instances+=1
        return pos_instances/self._labels.shape[0]
    def next_batch(self,batch_size):
        start=self._index_in_epoch
        self._index_in_epoch+=batch_size
        if self._index_in_epoch>self.num_instances:
        	self._epochs_completed+=1
        	#shuffle the data
        	shuffled=numpy.arange(self._num_instances)
        	numpy.random.shuffle(shuffled)
        	self._instances=self._instances[shuffled]
        	self._labels=self._labels[shuffled]
                if self._groups != []:
                        self._groups=self._groups[shuffled]
        	#start next epoch
        	start=0
        	self._index_in_epoch=batch_size
        end=self._index_in_epoch
        return self._instances[start:end],self._labels[start:end],self._groups[start:end]

'''
read from train and test files
@param train_file(string): path of training instance data
@param train_label_file(string): path of training data labels
@param test_file(string):path of test instance data
@param test_label_file(string): path of test data labels
@param group_file(string): group file for pairwise loss function
@return an object of DataSets
'''     
def read_data_sets(train_file,train_label_file,test_file, test_label_file, group_file):
	class DataSets(object):
		pass
	data_sets=DataSets()
	train_instances=readFile(train_file)
        train_labels=readFile(train_label_file)
	test_instances=readFile(test_file)
        test_labels=readFile(test_label_file)
        '''
        print("data read complete, train instance dimension is ",numpy.shape(train_instances),"label dimension is,",numpy.shape(train_labels))
        
        print("first features",train_instances[1,:34])
        print("second features",train_instances[1,34:174])
        print("third features",train_instances[1,174:211])
        print("fourth features",train_instances[1,-15:])
        print("fourth features",train_instances[1,211:])
        '''
        groups=readGroup(group_file)
        #print("group file dimension is,", numpy.shape(groups),groups)
        # normalization,optional
        #train_instances,test_instances=normalize(train_instances,test_instances)

        #Note: added for shuffling features
        if False:
                shuffled=numpy.arange(test_instances.shape[1])
                numpy.random.shuffle(shuffled)
                test_instances=test_instances[:,shuffled]
                train_instances=train_instances[:,shuffled]

	data_sets.train=DataSet(train_instances,train_labels, groups)
	data_sets.test=DataSet(test_instances,test_labels)
	return data_sets

'''
min-max normalization for training instance and test instance
@param train_instances: training instance data
@param test_instances: test instance data
@return nomalized training and test instances
'''  
def normalize(train_instances,test_instances):
        #train_size=train_instances.shape[0]
        train_min=train_instances.min(0)
        train_max=train_instances.max(0)
        train_instances=(train_instances-train_min)/(train_max-train_min)
        test_instances=(test_instances-train_min)/(train_max-train_min)
        return train_instances,test_instances
        #comb=numpy.r_[train_instances,test_instances]
        #comb=(comb-comb.min(0))/(comb.max(0)-comb.min(0))
        #return comb[0:train_size,:],comb[train_size:,:]

