#!/bin/bash

apt update
apt install -y openjdk-8-jdk
apt install -y maven

cd /DeepFaultLocalization/FeatureExtraction/UsefulTools
tar zxvf pitest.tar.gz
mkdir -p ~/.m2/repository/org/
cp -r pitest ~/.m2/repository/org/
unzip tools.zip

cd /DeepFaultLocalization/FeatureExtraction/
tar zxvf importantFolders.tar.gz

cd /DeepFaultLocalization/FeatureExtraction/UsefulData
tar zxvf defects4j.tar.gz
