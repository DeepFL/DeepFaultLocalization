#! /bin/bash
rootPath=$1
project=$2
version=$3
cd $rootPath/SubjectExample/$project/$version
echo "------------------------------------------------"
echo "Start spctrum raw feature"
echo "------------------------------------------------"
mvn clean test -f pomSpectrum.xml
mvn org.pitest:pitest-maven:mutationCoverage -f pomSpectrum.xml
cp line-assert/line-assert_detail $rootPath/RawFeatures/Spectrum/$project/$version.txt


echo "------------------------------------------------"
echo "Spctrum raw feature DONE......"
echo "------------------------------------------------"

