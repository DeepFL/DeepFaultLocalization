#! /bin/bash
rootPath=$1
project=$2
version=$3
cd $rootPath/Scripts/RawFeatures

sh SpectrumFeature.sh $rootPath $project $version

sh MutationFeature.sh $rootPath $project $version

sh ComplexityFeature.sh $rootPath $project $version

sh TextualFeature.sh $rootPath $project $version
