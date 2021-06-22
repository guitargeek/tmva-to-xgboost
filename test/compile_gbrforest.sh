#!/bin/bash

# Compiling GBR Forest from CMSSW

mkdir gbrforest

cp CMakeLists.txt gbrforest/

cd gbrforest

mkdir src
cd src
wget https://raw.githubusercontent.com/cms-sw/cmssw/master/CondFormats/GBRForest/src/GBRTree.cc
wget https://raw.githubusercontent.com/cms-sw/cmssw/master/CommonTools/MVAUtils/src/GBRForestTools.cc
wget https://raw.githubusercontent.com/cms-sw/cmssw/master/CommonTools/MVAUtils/src/TMVAZipReader.cc
cd ..

mkdir include
cd include
wget https://raw.githubusercontent.com/cms-sw/cmssw/master/CommonTools/MVAUtils/interface/GBRForestTools.h
wget https://raw.githubusercontent.com/cms-sw/cmssw/master/CommonTools/MVAUtils/interface/TMVAZipReader.h
wget https://raw.githubusercontent.com/cms-sw/cmssw/master/CondFormats/GBRForest/interface/GBRForest.h
wget https://raw.githubusercontent.com/cms-sw/cmssw/master/CondFormats/GBRForest/interface/GBRTree.h
cd ..

cd ..
patch -s -p0 < gbrforest.patch
cd gbrforest


mkdir build
cd build
cmake ..
make
cd ..

cd ..
