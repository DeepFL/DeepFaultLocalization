# DeepFL
DeepFL is a deep learning approach to automatically learn the most effective existing/latent features for precise fault localization. It exploits multi-dimensional features, i.e., spectrum-based, mutation-based, complexity-based (code metrics) and textual-similarity-based information and implements two multi-layer perceptron variants (MLP and MLP2), one recurrent neural networks variant (BiRNN) and two tailored MLP variants by [Tensorflow](https://www.tensorflow.org/). For evaluating its performance, DeepFl adopts the benchmark subjects from [Defects4j](https://github.com/rjust/defects4j), an open source repository which provides buggy versions and the corresponding fixed versions of multiple projects.

## Data Preparation 
To simplify the testing process of DeepFL, we provide a docker container which can be downloaded from an online [Cloud Drive](https://mega.nz/#!HDwHEKbR!rv4BLiuzYnMsGUIn_W8YftGSy1AfuwHDez6h5IO0T1k). Please note that the size of our docker container including the dataset is larger than 30GB, so please prepare sufficient disk space.

If you are not familiar with docker or downloading the docker image is time-consuming in your network, you can also run DeepFL on your local machine: [Github](https://github.com/DeepFL/DeepFaultLocalization).

We have tested all commands below on Windows10, MacOS and Linux with docker version 2.0.0.3. Specially, Linux may require sudo permission for some commands.

We firstly un-compress the downloaded zip file to an available docker container:

```
unzip -o deepfl.zip
```

The commands to load the container as a new image are as followings.

 ```
docker import deepfl.tar tmp/deepfl
 ```

Next run the image and go to the bash terminal of docker container using:

```
docker run -t -i tmp/deepfl bash
```

Then go to the destination directory in docker container:

```
$cd DeepFaultLocalization
```

Then, pull some updates:


```
$ git pull
```

## Running DeepFL

The command to run DeepFL for each version is as follows:

A simple example, which trains the *dfl1* model using softmax as loss function for version 1 of Time subject, can be:

```
$python main.py . /result Time 1 dfl1 DeepFL softmax 2 1
```

A complete command to run DeepFL should be:

```
$python main.py . /result $subject $version $model $tech $loss $epoch $dump_step
```

Each parameter can be explained as follows:

1. First argument: `. ` : the absolute path of the parent directory including all datasets,
2. Second argument: `/result` :  the directory of the results. It should be noted that this argument cannot be changed to `/prepared_result`, which will overwrite our prepared result. 
3. $subject: the subject name, i.e. *Time*, *Chart*, *Lang*, *Math*, *Mockito* or *Closure*.
4. $version: the version number of the subject. Note that, the maximum numbers of the subjects above are 27, 26, 65, 106, 38, 133, respectively.
5. $model: the implemented model name, i.e., *mlp*, *mlp2*, *rnn*, *birnn*, *dfl1*, *dfl2*, *dfl1-Metrics*, *dfl1-Mutation*, *dfl1-Spectrum*, *dfl1-Textual* representing multi-layer perceptron with one hidden layer, multi-layer perceptron with two hidden layers, recurrent neural network, bidirectional recurrent neural network, tailored MLP1, tailored MLP2, and tailored MLP1 without the information of metrics, mutation, spectrum, textual similarity respectively.
6. $tech: the different dimensions of features corresponding to the dataset name, i.e., *DeepFL*, *CrossDeepFL* , *CrossValidation*, specially, both *CrossDeepFL* and *CrossValidation* can only use *softmax* as loss function.
7. $loss: the loss function name, i.e., *softmax*, *epairwise*.
8. $epoch: the number of training epochs.
9. $dump_step: the interval number of the epoch in which the results are stored into the result file. For example, if \$dump_step = 10, the results in epochs 10, 20, 30... are written into the files.

Please note that *CrossValidation* is slightly different from the others since the dataset of all the subjects has been mixed and then splitted into 10-fold. To efficiently use the command above, just set the parameter \$subject as "10fold", ​\$version as 1 to 10, and $tech as *CrossValidation*. Also, please only use *dfl_2* model and *softmax* loss function to run on *CrossValidation* according to our paper.

## Result Analysis

Since running DeepFL can be very time-consuming, we also provide result analysis scripts to generate the corresponding table/figures in the paper directly by analyzing the cached results from our prior runs. Specially, we prepared a cached result directory ./CachedResults to avoid time-consuming DeepFL reexecution.

Firstly go to `ResultAnalysis` directory:

```cmd
cd ResultAnalysis
```

The command to analyze the results for each RQ in the paper is as follows:

```
python result_main.py $RQ 55 ../CachedResults ../
```

Each parameter can be explained as follows:

1. $RQ: the corresponding RQ in the paper. There are 4 RQs but 5 kinds of results in total, i.e., *RQ1(Table 4), RQ2(Table 5), RQ2_2(Table 6), RQ3(Figure 8) and RQ4(Figure 9).*
2. iteration, which should be 55 in our paper
3. result directory (we use the prepared results here)
4. The absolute path of the parent directory including all the datasets

RQ1 and RQ2 output the corresponding tables in csv format in current directory `/ResultAnalysis`while RQ3 and RQ4 output a corresponding pdf file, which is located in sub-directory `/ResultAnalysis/Rdata`. 





