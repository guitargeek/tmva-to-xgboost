# TMVA to XGBoost

C++ executable to convert a TMVA BDT model stored in an XML file to a XGBoost JSON file.

## Compiling

The tmva2xgboost script has no dependencies other than the C++ standard library and the [boost libraries](https://www.boost.org/).

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

For some TMVA trainings, the response needs to be normalized by the number of trees. This can be enabled with the `--norm` options.
Sometimes, one also has to get the score from the `purity` XML attribute instead of the one names `res`. This can be done with the `--use_purity` option.

Some TMVA trainings also have a different boosting weight for each tree. these trainings are not supported by XGBoost.

### Note about sub-methods

It can also happen that a TMVA weight file contains multiple BDTs. You have to split it up manually in individual XML files with the correct structure. Please keep in mind that the numer of features can be different for each sub-method.

## Testing and validation

The tests are implemented as Python unit tests in the `test` directory.

To run the tests, change into the `test` directory and first compile the `gbrforest` library taken from CMSSSW that is used to produce the reference results in one of the unit tests:
```bash
cd test
sh compile_gbrforest.sh
```

Now, you can run the unit tests in the `test_*.py` files either py running them individually with Python, or by running `pytest` in the directory (or just `python -m unittest` if `pytest` is not installed).

There are two unit tests that are also useful to see how the converted XGBoost model can be used to reproduce the output of TMVA or GBRForest.
