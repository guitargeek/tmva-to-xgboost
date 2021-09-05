# Authors:
# * Massimiliano Galli 09/2021
# * Jonas Rembser 09/2021

import unittest
import sys


class TestPhotonIDTMVA(unittest.TestCase):

    model_name = "PhoID_barrel_UL2017_GJetMC_SATrain_nTree2k_LR_0p1_13052020_BDTG"
    n_features = 12

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

        # download data file
        url = "https://github.com/maxgalli/UsefulHEPScripts/raw/master/flashgg_investigate/compare_nano_micro/tmva_xgboost_reproducers/lead_processed_nano.root"
        r = requests.get(url)
        with open("lead_processed_nano.root", "wb") as f:
            f.write(r.content)

        # decompress the gzipped model file
        with gzip.open("res/" + model_name + ".weights.xml.gz", "rb") as f_in:
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

    def tearDown(self):
        import os

        os.remove(self.model_name + ".weights.xml")
        os.remove(self.model_name + ".json")
        os.remove("lead_processed_nano.root")

    def test_electronid_gbrforest(self):
        import ROOT
        import numpy as np
        import xgboost
        from array import array
        import awkward as ak
        import uproot

        processed_nano = "lead_processed_nano.root"

        xml_model = self.model_name + ".weights.xml"
        xgb_model = self.model_name + ".json"

        entry_stop = 10000

        # Explicitely recompute also XGBoost
        f = uproot.open(processed_nano)
        t = f["Events"]
        events = t.arrays(entry_stop=entry_stop)

        var_order = list(events.fields)
        bdt_inputs = np.column_stack([ak.to_numpy(events[name]) for name in var_order])

        mva = xgboost.Booster()
        mva.load_model(xgb_model)
        tempmatrix = xgboost.DMatrix(bdt_inputs, feature_names=var_order)
        lead_idmva_xgboost = mva.predict(tempmatrix)

        # You need to do this transformation on the raw xgboost predictions to
        # get values between -1 and 1 that match the TMVA scores.
        lead_idmva_xgboost = 1 - 2.0 / (1 + np.exp(2 * lead_idmva_xgboost))

        f = ROOT.TFile(processed_nano)
        events = f.Get("Events")

        ROOT.TMVA.Tools.Instance()
        ROOT.TMVA.PyMethodBase.PyInitialize()
        reader = ROOT.TMVA.Reader("Color:!Silent")

        branches = {}
        for branch in events.GetListOfBranches():
            branch_name = branch.GetName()
            branches[branch_name] = array("f", [-999])
            reader.AddVariable(branch_name, branches[branch_name])
            events.SetBranchAddress(branch_name, branches[branch_name])

        reader.BookMVA("BDT", ROOT.TString(xml_model))

        n_entries = min(entry_stop, events.GetEntries())
        lead_idmva_tmva = np.zeros(n_entries)
        for i in range(n_entries):
            events.GetEntry(i)
            lead_idmva_tmva[i] = reader.EvaluateMVA("BDT")

        np.testing.assert_almost_equal(lead_idmva_xgboost, lead_idmva_tmva, decimal=5)


if __name__ == "__main__":
    unittest.main()
