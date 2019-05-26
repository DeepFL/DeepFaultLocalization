#! /bin/bash
rootPath=$1

# get Spectrum and mutation
cd $rootPath/FeatureValues/Spectrum/Chart
mkdir 1
cd $rootPath/UsefulTools/mutationfl/bin
java Main.RunMain mutation 1 Chart $rootPath
cd $rootPath/UsefulTools/
python convertSpecAndMutation.py $rootPath

# get source code complexity values
cd $rootPath/UsefulTools/mutationfl/bin
java Main.RunMain MetricData 1 Chart $rootPath

# just copy byte code complexity values from RawFeatures
cp $rootPath/RawFeatures/Complexity/Chart/1byte.txt $rootPath/FeatureValues/Complexity/Chart/

#get Textual features values
cd $rootPath/UsefulTools/Textual
javac GetSusFromInformR.java
java GetSusFromInformR 1 Chart $rootPath
python WriteIRData.py Chart 1 $rootPath

