import cppyy
import xgboost
import numpy as np
import pandas as pd
import sys

args = sys.argv[1:]

model_name = args[0]
n_features = int(args[1])

cppyy.include("gbrforest/include/GBRForestTools.h")
cppyy.load_library("gbrforest/build/libgbrforest.so")
cppyy.load_library("/usr/lib/libtinyxml2.so")

gbr = cppyy.gbl.createGBRForest(model_name + ".weights.xml")
xgb = xgboost.XGBClassifier()
xgb.load_model(model_name + ".json")

n_entries = 10

X = np.array(np.random.normal(loc=0, scale=1, size=(n_entries, n_features)), dtype=np.float32)

score_xgb = xgb.predict_proba(X)[:, 1]
score_gbr = np.array([gbr.GetResponse(X[i]) for i in range(n_entries)])

print("")
print("Comparing electron MVA scores predicted by XGBoost and GBRForest reference:")
print("")

df = pd.DataFrame({"score_xgb": score_xgb, "score_gbr": score_gbr})

print(df)
