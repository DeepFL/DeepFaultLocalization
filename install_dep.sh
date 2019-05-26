#!/bin/bash

apt update
apt install openjdk-8-jdk
apt install maven

cd FeatureExtraction/UsefulTools
tar zxvf pitest.tar.gz
cp -r pitest /root/.m2/repository/org/
unzip tools.zip

cd ..
tar zxvf importantFolders.tar.gz
