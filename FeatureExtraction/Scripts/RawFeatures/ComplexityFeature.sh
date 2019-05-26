#! /bin/bash
rootPath=$1
project=$2
version=$3

echo "------------------------------------------------"
echo "Start complexity raw feature"
echo "------------------------------------------------"



cd $rootPath/UsefulTools/JHawkAcademic/CommandLine/
Dir=$rootPath/SubjectExample/$project/$version/src
java -jar JHawkCommandLine.jar -f .*\.java -r -l m -s $Dir -x $rootPath/RawFeatures/Complexity/$project/$version -system MySystem

cd $rootPath/SubjectExample/$project/$version/
java -jar $rootPath/UsefulTools/bytecode-metrics/target/bytecode-metrics-0.0.1-SNAPSHOT.jar ./target $rootPath/RawFeatures/Complexity/$project/$version'byte.txt'

echo "------------------------------------------------"
echo "Complexity raw feature DONE...."
echo "------------------------------------------------"

