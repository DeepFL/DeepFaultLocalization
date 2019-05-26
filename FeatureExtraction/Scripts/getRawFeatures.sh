#! /bin/bash
rootPath=$1

cd $rootPath/Scripts/RawFeatures

sh SpectrumFeature.sh $rootPath

sh MutationFeature.sh $rootPath

sh ComplexityFeature.sh $rootPath

sh TextualFeature.sh $rootPath
