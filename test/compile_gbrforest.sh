#!/bin/bash

# Compiling GBR Forest from CMSSW
cmssw_version=CMSSW_12_0_1

mkdir gbrforest

cp CMakeLists.txt gbrforest/

cd gbrforest

mkdir src
cd src
wget -q https://raw.githubusercontent.com/cms-sw/cmssw/$cmssw_version/CondFormats/GBRForest/src/GBRTree.cc
wget -q https://raw.githubusercontent.com/cms-sw/cmssw/$cmssw_version/CommonTools/MVAUtils/src/GBRForestTools.cc
wget -q https://raw.githubusercontent.com/cms-sw/cmssw/$cmssw_version/CommonTools/MVAUtils/src/TMVAZipReader.cc
cd ..

mkdir include
cd include
wget -q https://raw.githubusercontent.com/cms-sw/cmssw/$cmssw_version/CommonTools/MVAUtils/interface/GBRForestTools.h
wget -q https://raw.githubusercontent.com/cms-sw/cmssw/$cmssw_version/CommonTools/MVAUtils/interface/TMVAZipReader.h
wget -q https://raw.githubusercontent.com/cms-sw/cmssw/$cmssw_version/CondFormats/GBRForest/interface/GBRForest.h
wget -q https://raw.githubusercontent.com/cms-sw/cmssw/$cmssw_version/CondFormats/GBRForest/interface/GBRTree.h
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
