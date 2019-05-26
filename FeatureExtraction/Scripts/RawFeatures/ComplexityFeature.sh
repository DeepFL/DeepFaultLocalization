#! /bin/bash
rootPath=$1
cd $rootPath/UsefulTools/JHawkAcademic/CommandLine/
Dir=$rootPath/SubjectExample/Chart/1/src
java -jar JHawkCommandLine.jar -f .*\.java -r -l m -s $Dir -x $rootPath/RawFeatures/Complexity/Chart/1 -system MySystem

cd $rootPath/SubjectExample/Chart/1/
java -jar $rootPath/UsefulTools/bytecode-metrics/target/bytecode-metrics-0.0.1-SNAPSHOT.jar ./target $rootPath/RawFeatures/Complexity/Chart/1byte.txt

