# Authors:
# * Jonas Rembser 09/2021

import unittest
import warnings
import sys

with warnings.catch_warnings():
    warnings.simplefilter("ignore")
    import cppyy


class TestElectronIDGBRForest(unittest.TestCase):

    model_name = "electronID_mva_Spring16_GeneralPurpose_V1_EB1_10"
    n_features = 22

    @staticmethod
    def _find_tmva2xgboost_executable():
        from os.path import exists

        for path in ["./tmva2xgboost", "../tmva2xgboost"]:
            if exists(path):
                return path
        return "tmva2xgboost"

    def setUp(self):
        import requests
        import gzip
        import shutil
        import os

        model_name = self.model_name

        # download gzipped model file
        url = (
            "https://github.com/cms-data/RecoEgamma-ElectronIdentification/raw/master/Spring16_GeneralPurpose_V1/"
            + model_name
            + ".weights.xml.gz"
        )
        r = requests.get(url)
        with open(model_name + ".weights.xml.gz", "wb") as f:
            f.write(r.content)

        # decompress the gzipped model file
        with gzip.open(model_name + ".weights.xml.gz", "rb") as f_in:
            with open(model_name + ".weights.xml", "wb") as f_out:
                shutil.copyfileobj(f_in, f_out)

        # convert the model to xgboost
        os.system(
            self._find_tmva2xgboost_executable()
            + " --input "
            + model_name
            + ".weights.xml --n_features "
            + str(self.n_features)
            + " > "
            + model_name
            + ".json"
        )

        cppyy.include("gbrforest/include/GBRForestTools.h")
        cppyy.load_library("gbrforest/build/libgbrforest.so")
        cppyy.load_library("/usr/lib/libtinyxml2.so")

    def tearDown(self):
        import os

        os.remove(self.model_name + ".weights.xml.gz")
        os.remove(self.model_name + ".weights.xml")
        os.remove(self.model_name + ".json")

    def test_electronid_gbrforest(self):
        import xgboost
        import numpy as np

        model_name = self.model_name

        n_samples = 1000

        gbr = cppyy.gbl.createGBRForest(model_name + ".weights.xml")
        xgb = xgboost.XGBClassifier()
        xgb = xgboost.Booster()
        xgb.load_model(model_name + ".json")

        x = np.array(np.random.normal(loc=0, scale=1, size=(n_samples, self.n_features)), dtype=np.float32)
        x_dmatrix = xgboost.DMatrix(x)

        score_xgb = xgb.predict(x_dmatrix)
        score_gbr = np.array([gbr.GetResponse(x[i]) for i in range(n_samples)])

        # Results will slightly differ because the GBRForest doesn't use double
        # precision. That's why we use `almost_equal` here.
        np.testing.assert_array_almost_equal(score_xgb, score_gbr, decimal=4)


if __name__ == "__main__":
    unittest.main()
