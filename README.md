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
cd DeepFaultLocalization
```

In the directory, we provide a prepared result directory `./paperResult` to avoid time-consuming training.

The command to run DeepFL for each version is as follows:


```
$git pull
```

```
$python main.py . /result $subject $version $model $tech $loss $epoch $dump_step
```
Each parameter can be explained as follows:
1. First argument: `. ` : The absolute path of the parent directory including all datasets,
2. Second argument: `/result` :  The directory of the results. It should be noted that this argument cannot be changed to `/prepared_result`, which will overwrite our prepared result. 
3. $subject: The subject name, which can be *Time*, *Chart*, *Lang*, *Math*, *Mockito* or *Closure*.
4. $version: The version number of the subject. Note that, the maximum numbers of subjects above are 27, 26, 65, 106, 38, 133, respectively.
5. $model: The implemented model name, which can be *mlp*, *mlp2*, *rnn*, *birnn*, *dfl1*, *dfl2*, *dfl1-Metrics*, *dfl1-Mutation*, *dfl1-Spectrum*, *dfl1-Textual* representing multi-layer perceptron with one hidden layer, multi-layer perceptron with two hidden layers, recurrent neural network, bidirectional recurrent neural network, tailored MLP1, tailored MLP2, and tailored MLP1 without information of metrics, mutation, spectrum, textual similarity respectively.
6. $tech: The different dimensions of features, corresponding to the name of dataset, can be *DeepFL*, *CrossDeepFL* , *CrossValidation*.
7. $loss: The name of loss function, which can be *softmax*, *epairwise*.
8. $epoch: The number of training epochs.
9. \$dump_step: The interval number of epoch in which the result will be stored into the result file. For example, if $dump_step = 10, the results in epochs 10, 20, 30... will be written into the files.

Please note that *CrossValidation* is slightly different with others since the dataset of all subjects has been mixed and then splitted into 10-fold. To easily use the command above, just set the parameter subject as "10fold", \$version as 1 to 10, and $tech as "CrossValidation". Also, please only use *mlp_dfl_2* model and *softmax* loss function to run on *CrossValidation* according to the research question in our paper.

## Script

Cause there are kinds of subjects and many versions, we therefore write some scripts for you to train and evaluate the whole project quickly. It should be noted that many arguments such as dataset directory and output directory are predefined in corresponding script.

It should be noted that the script we prepared predefines the iteration as 2 for quicker running, if you want to get a comparable results with paper, the `$iter` parameter should be modified to 55. 

The first script `run_deepfl.sh` will run almost all of the cases in our paper except  *CrossValidation*  tech:

```
./run_deepfl.sh
```

Also, for *CrossValidation*  tech, another script should be used:

```
./10fold.sh
```

## Result Analysis

We provide result analysis scripts to generate corresponding table/figures in papers.

Firstly go to `ResultAnalysis` directory:

```cmd
cd ResultAnalysis
```

The command to analyze results for each RQ in paper is as follows:

```
python result_main.py $RQ 55 ../paperResult/ ../
```

Each parameter can be explained as follows:

1. $RQ: corresponding RQ in papers, there are 4 RQs  totally, which should be *RQ1, RQ2, RQ3 and RQ4.*
2. iteration, which should be 55 in our paper
3. result directory, we use prepared results here
4. The absolute path of the parent directory including all datasets