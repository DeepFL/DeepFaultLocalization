#! /bin/bash
rootPath=$1
cd $rootPath/SubjectExample/Chart/1
mvn org.pitest:pitest-maven:mutationCoverage -f pomSpectrum.xml
cp line-assert/line-assert_detail $rootPath/RawFeatures/Spectrum/Chart/1.txt
