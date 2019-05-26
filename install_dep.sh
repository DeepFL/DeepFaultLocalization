#!/bin/bash

apt update
apt install -y openjdk-8-jdk
apt install -y maven

cd FeatureExtraction/UsefulTools
tar zxvf pitest.tar.gz
cp -r pitest ~/.m2/repository/org/
unzip tools.zip

cd ..
tar zxvf importantFolders.tar.gz

cd UsefulData
tar zxvf defects4j.tar.gz
