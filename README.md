# TMVA to XGBoost

C++ executable to convert a TMVA BDT model stored in an XML file to a XGBoost JSON file.

## Compiling

The tmva2xgboost script has no dependencies other than the C++ standard library.

```bash
g++ -o tmva2xgboost -lboost_program_options tmva2xgboost.cpp
```

## Usage

You have to pass the XML model file path and the number of features used by the model, for example:

```bash
./tmva2xgboost --input model.weights.xml --n_features 22 > model.json
```

**Always validate your converted XGBoost model before using it in production!**

### Note about options

For some TMVA trainings, the response needs to be normalized by the number of trees. This can be enables with the `--norm` options.
Sometimes, one also has to get the score from the `purity` XML attribute instead of the one names `res`. This can be done with the `--do-purity` option.

Some TMVA trainings also have a different boosting weight for each tree. these trainings are not supported by XGBoost.

### Note about sub-methods

It can also happen that a TMVA weight file contains multiple BDTs. You have to split it up manually in individual XML files with the correct structure. Please keep in mind that the numer of features can be different for each sub-method.

## Testing and validation

To use the testing scripts, copy the `tmva2xgboost` executable into the test directory and change into the test directory.

There is one test that validates the converted XGBoost model at the example of the electron MVA used in CMSSSW. The reference results are produced with the GBForest as in CMSSW, but using random values as input.

In the test directory, you can compile the GBRForest tools and run the comparison as follows (the `tmva2xgboost` executable has to be in the test diretory too):

```bash
sh compile_gbrforest.sh
sh run_electron_mva_test.sh
```
