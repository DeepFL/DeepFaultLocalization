#! /bin/bash
rootPath=$1
project=$2
version=$3
echo "------------------------------------------------"
echo "Start mutation raw feature"
echo "------------------------------------------------"


cd $rootPath/SubjectExample/$project/$version
mvn org.pitest:pitest-maven:mutationCoverage -f pomMutation.xml

rm -rf $rootPath/RawFeatures/Mutation/$project/mutation-test/$version.txt
rm -rf $rootPath/RawFeatures/Mutation/$project/coverage-test/$version.txt
cd mutation-test
for file in *.gz; do gzip -c -d "$file">>$rootPath/RawFeatures/Mutation/$project/mutation-test/$version.txt; done
cd ../coverage-test
for file in *; do data=`cat $file`; if ! [ -z "$data" ]; then cat "$file">>$rootPath/RawFeatures/Mutation/$project/coverage-test/$version.txt; echo "^^^^^^">>$rootPath/RawFeatures/Mutation/$project/coverage-test/$version.txt; fi done

echo "------------------------------------------------"
echo "Mutation raw feature DONE...."
echo "------------------------------------------------"

