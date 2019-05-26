#! /bin/bash
rootPath=$1
project=$2
version=$3

# get Spectrum and mutation
echo "Extracting spctrum and mutation features......"
rm -rf $rootPath/FeatureValues/tempResult/$project/$version/*.txt
rm -rf $rootPath/FeatureValues/Spectrum/$project/$version.txt
rm -rf $rootPath/FeatureValues/Mutation/$project/$version.txt
cd $rootPath/UsefulTools/mutationfl/bin
java Main.RunMain mutation $version $project $rootPath
cd $rootPath/UsefulTools/
python convertSpecAndMutation.py $rootPath

# get source code complexity values
echo "Extracting complexity features......"
rm -rf $rootPath/FeatureValues/Complexity/$project/$version.txt
rm -rf $rootPath/FeatureValues/Complexity/$project/$version'source.txt'
rm -rf $rootPath/FeatureValues/Complexity/$project/$version'byte.txt'
cd $rootPath/UsefulTools/mutationfl/bin
java Main.RunMain MetricData $version $project $rootPath

# just copy byte code complexity values from RawFeatures
cp $rootPath/RawFeatures/Complexity/$project/$version'byte.txt' $rootPath/FeatureValues/Complexity/$project/

# combine complexity values
cd $rootPath/UsefulTools
python combineMetric.py $rootPath 


#get Textual features values
echo "Extracting textual features......"
rm -rf $rootPath/FeatureValues/Textual/$project/$version/tempResults/*.txt
rm -rf $rootPath/FeatureValues/Textual/$project/$version.txt
cd $rootPath/UsefulTools/Textual
javac GetSusFromInformR.java
java GetSusFromInformR $version $project $rootPath
python WriteIRData.py $project $version $rootPath

