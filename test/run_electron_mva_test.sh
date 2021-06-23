#!/bin/bash

# Download Electron MVA XML file and run test

wget https://github.com/cms-data/RecoEgamma-ElectronIdentification/raw/master/Spring16_GeneralPurpose_V1/electronID_mva_Spring16_GeneralPurpose_V1_EB1_10.weights.xml.gz
gzip -d electronID_mva_Spring16_GeneralPurpose_V1_EB1_10.weights.xml.gz

./tmva2xgboost --input electronID_mva_Spring16_GeneralPurpose_V1_EB1_10.weights.xml --n_features 22 > electronID_mva_Spring16_GeneralPurpose_V1_EB1_10.json
mv electronID_mva_Spring16_GeneralPurpose_V1_EB1_10.weights.xml electronID_mva_Spring16_GeneralPurpose_V1_EB1_10.xml

python run_gbr_comparison.py electronID_mva_Spring16_GeneralPurpose_V1_EB1_10 22

rm electronID_mva_Spring16_GeneralPurpose_V1_EB1_10*
