#! /bin/bash
rootPath=$1

cd $rootPath/SubjectExample/Chart/1
mvn org.pitest:pitest-maven:mutationCoverage -f pomMutation.xml

rm -rf $rootPath/RawFeatures/Mutation/Chart/mutation-test/1.txt
rm -rf $rootPath/RawFeatures/Mutation/Chart/coverage-test/1.txt
cd mutation-test
for file in *.gz; do gzip -c -d "$file">>$rootPath/RawFeatures/Mutation/Chart/mutation-test/1.txt; done
cd ../coverage-test
for file in *; do data=`cat $file`; if ! [ -z "$data" ]; then cat "$file">>$rootPath/RawFeatures/Mutation/Chart/coverage-test/1.txt; echo "^^^^^^">>$rootPath/RawFeatures/Mutation/Chart/coverage-test/1.txt; fi done

