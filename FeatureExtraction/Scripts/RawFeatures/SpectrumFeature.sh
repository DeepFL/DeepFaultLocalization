#! /bin/bash
rootPath=$1
project=$2
version=$3
cd $rootPath/SubjectExample/$project/$version
mvn clean test -f pomSpectrum.xml
mvn org.pitest:pitest-maven:mutationCoverage -f pomSpectrum.xml
cp line-assert/line-assert_detail $rootPath/RawFeatures/Spectrum/$project/$version.txt
