#! /bin/bash
rootPath=$1
cd $rootPath/UsefulTools/PIT-Spectrum
mvn clean install -Dmaven.test.skip=true
cd $rootPath/SubjectExample/Chart/1
mvn clean test
mvn org.pitest:pitest-maven:mutationCoverage -f pomSpectrum.xml
cp line-assert/line-assert_detail $rootPath/RawFeatures/Spectrum/Chart/1.txt
