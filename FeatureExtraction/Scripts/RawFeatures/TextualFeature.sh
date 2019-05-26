#! /bin/bash
rootPath=$1
# 1. clear previous data
project=$rootPath'RawFeatures/Textual/Chart/1'
for query in $project/*
do
rm -rf $query/IndexPara
rm -rf $query/queryPara
rm -rf $query/Result.txt
rm -rf $query/Documents/*
rm -rf $query/Index/*
done


#2. Prepare data for Indri
java -classpath $rootPath/UsefulTools/mutationfl/lib/\*:$rootPath/UsefulTools/mutationfl/bin/  Main.RunMain InformationR 1 Chart $rootPath

#3. Run Indri

querys="McltoT McltoTc McltoTFS McomtoT McomtoTc McomtoTFS MmtoT MmtoTc MmtoTFS MtoT MtoTc MtoTFS MvtoT MvtoTc MvtoTFS"
for quer in $querys
do
cd $rootPath/RawFeatures/Textual/Chart/1/$quer/Documents
sed -i 's/>/>'\\n'/g' *.txt
cd -
$rootPath/UsefulTools/indri-5.11/buildindex/IndriBuildIndex $project/$quer/IndexPara

$rootPath/UsefulTools/indri-5.11/runquery/IndriRunQuery $project/$quer/queryPara  -baseline=tfidf,k1:1,b:0.3>>$project/$quer/Result.txt

done

