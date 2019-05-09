# DeepFL
DeepFL is a deep-learning-based fault localization technique. It implements two multi-layer perceptron variants (MLP and MLP2), one recurrent neural networks variant (BiRNN) and two tailored MLP variants by [Tensorflow](https://www.tensorflow.org/). The benchmark subject is from [Defects4j](https://github.com/rjust/defects4j), which is an open source repository, providing some buggy versions and corresponding fixed versions of different projects. Features of the dataset include different dimensions, e.g., spectrum-based, mutation-based, complexity-based (code metrics) and textual-similarity-based information. 

## Data Collection
Since collecting the feature data is complicated and time consuming, we just provide the cleaned dataset used for learning process. Please refer to Section 4.1 in our paper for implementation details.

Next, we focus on introducing the process of deep learning and result analysis.


## Running DeepFL

To easy test our project, we provide a docker container which can be downloaded from an online [Cloud Drive](http..).
The commands to load the container as a new image are as followings.

 ```
docker import deepfl.tar tmp/deepfl 
 ```

Then run the image using:

```
docker run -t -i tmp/deepfl 
```

Then go to the destination directory:

```
cd home/DeepFaultLocalization
```

The command to run DeepFL for each version is as follows:


```
$ git pull
```

```
$python main.py . /result $subject $version $model $tech $loss $epoch $dump_step
```
Each parameter can be explained as follows:
1. First argument: `. ` : The absolute path of the parent directory including all datasets, for example, if the dataset is DeepFL, its directory can be /home/DeepLearningData/DeepFL ("/home/DeepLearningData/" is created by users, and "DeepFL" is put into it)
2. Second argument: `/result` :  The directory of the results. It should be noted that this argument cannot be changed to `/prepared_result`, which will overwrite our prepared result. 
3. $subject: The subject name, which can be *Time*, *Chart*, *Lang*, *Math*, *Mockito* or *Closure*.
4. $version: The version number of the subject. Note that, the maximum numbers of subjects above are 27, 26, 65, 106, 38, 133, respectively.
5. $model: The implemented model name, which can be *mlp*, *mlp2*, *rnn*, *birnn*, *dfl1*, *dfl2*, *dfl1-Metrics*, *dfl1-Mutation*, *dfl1-Spectrum*, *dfl1-Textual* representing multi-layer perceptron with one hidden layer, multi-layer perceptron with two hidden layers, recurrent neural network, bidirectional recurrent neural network, tailored MLP1, tailored MLP2, and tailored MLP1 without information of metrics, mutation, spectrum, textual similarity respectively.
6. $tech: The different dimensions of features, corresponding to the name of dataset, can be *DeepFL*, *CrossDeepFL* , *CrossValidation*.
7. $loss: The name of loss function, which can be *softmax*, *epairwise*.
8. $epoch: The number of training epochs.
9. \$dump_step: The interval number of epoch in which the result will be stored into the result file. For example, if $dump_step = 10, the results in epochs 10, 20, 30... will be written into the files.

Please note that *CrossValidation* is slightly different with others since the dataset of all subjects has been mixed and then splitted into 10-fold. To easily use the command above, just set the parameter $subject as "10fold", $version as 1 to 10, and $tech as "CrossValidation". Also, please only use *mlp_dfl_2* model and *softmax* loss function to run on *CrossValidation* according to the research question in our paper.

## Script

Cause there are kinds of subjects and many versions, we therefore write some scripts for you to train and evaluate the whole project quickly. It should be noted that many arguments such as dataset directory and output directory are predefined in corresponding script.

```
./test.sh $subject $epoch
```

Each parameter can be explained as follows:

1. $subject: The subject name, which can be *all* (train all of subjects above) *Time*, *Chart*, *Lang*, *Math*, *Mockito* or *Closure*.
2. $epoch: The number of training epochs.

Also, for *CrossValidation*  tech, another script should be used:

```
./10fold.sh
```

## Result Analysis



<!---

## Results statistics ##

After running all subject versions, run the following command to calculate the five measurements Top-1, Top-3, Top-5, MFR, MAR:

```
python rank_parser.py /absolute/path/to/ParentDirofDataset /absolute/path/to/Result $tech $model $loss $epoch
```
Please note that due to the randomly initialized parameters, the results may be slightly different from our paper.

-->

