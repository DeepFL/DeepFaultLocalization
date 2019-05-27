#! /bin/bash
rootPath=$1
project=$2
version=$3

# get Spectrum and mutation
echo "Extracting final spectrum and mutation features......"
rm -rf $rootPath/FinalFeatures/tempResult/$project/$version/*.txt
rm -rf $rootPath/FinalFeatures/Spectrum/$project/$version.txt
rm -rf $rootPath/FinalFeatures/Mutation/$project/$version.txt
cd $rootPath/UsefulTools/mutationfl/bin
java Main.RunMain mutation $version $project $rootPath
cd $rootPath/UsefulTools/
python convertSpecAndMutation.py $rootPath

# get source code complexity values
echo "Extracting final complexity features......"
rm -rf $rootPath/FinalFeatures/Complexity/$project/$version.txt
rm -rf $rootPath/FinalFeatures/Complexity/$project/$version'source.txt'
rm -rf $rootPath/FinalFeatures/Complexity/$project/$version'byte.txt'
cd $rootPath/UsefulTools/mutationfl/bin
java Main.RunMain MetricData $version $project $rootPath

# just copy byte code complexity values from RawFeatures
cp $rootPath/RawFeatures/Complexity/$project/$version'byte.txt' $rootPath/FinalFeatures/Complexity/$project/

# combine complexity values
cd $rootPath/UsefulTools
python combineMetric.py $rootPath 


#get Textual features values
echo "Extracting final textual features......"
rm -rf $rootPath/FinalFeatures/Textual/$project/$version/tempResults/*.txt
rm -rf $rootPath/FinalFeatures/Textual/$project/$version.txt
cd $rootPath/UsefulTools/Textual
javac GetSusFromInformR.java
java GetSusFromInformR $version $project $rootPath
python WriteIRData.py $project $version $rootPath

