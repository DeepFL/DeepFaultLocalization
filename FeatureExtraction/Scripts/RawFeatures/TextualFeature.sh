#! /bin/bash
rootPath=$1
project=$2
version=$3

echo "------------------------------------------------"
echo "Start textual raw feature"
echo "------------------------------------------------"



# 1. clear previous data
pro=$rootPath'RawFeatures/Textual/'$project/$version
for query in $pro/*
do
rm -rf $query/IndexPara
rm -rf $query/queryPara
rm -rf $query/Result.txt
rm -rf $query/Documents/*
rm -rf $query/Index/*
done


#2. Prepare data for Indri
java -classpath $rootPath/UsefulTools/mutationfl/lib/\*:$rootPath/UsefulTools/mutationfl/bin/  Main.RunMain InformationR $version $project $rootPath

#3. Run Indri

querys="McltoT McltoTc McltoTFS McomtoT McomtoTc McomtoTFS MmtoT MmtoTc MmtoTFS MtoT MtoTc MtoTFS MvtoT MvtoTc MvtoTFS"
for quer in $querys
do
cd $rootPath/RawFeatures/Textual/$project/$version/$quer/Documents
sed -i 's/>/>'\\n'/g' *.txt
chmod -R 777 $rootPath/UsefulTools/indri-5.11
$rootPath/UsefulTools/indri-5.11/buildindex/IndriBuildIndex $pro/$quer/IndexPara

$rootPath/UsefulTools/indri-5.11/runquery/IndriRunQuery $pro/$quer/queryPara  -baseline=tfidf,k1:1,b:0.3>>$pro/$quer/Result.txt

done


echo "------------------------------------------------"
echo "Textual raw feature DONE...."
echo "------------------------------------------------"

