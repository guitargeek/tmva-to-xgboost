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

gbr = cppyy.gbl.createGBRForest(model_name + ".xml")
xgb = xgboost.XGBClassifier()
xgb.load_model(model_name + ".json")

n_entries = 10000

X = np.array(np.random.normal(loc=0, scale=1, size=(n_entries, n_features)), dtype=np.float32)

score_xgb = xgb.predict_proba(X)[:, 1]
score_gbr = np.array([gbr.GetResponse(X[i]) for i in range(n_entries)])
# score_gbr_gradboost = np.array([gbr.GetGradBoostClassifier(X[i]) for i in range(n_entries)])

print("")
print("Comparing electron MVA scores predicted by XGBoost and GBRForest reference.")
print("Results will slightly differ because the GBRForest doesn't use double precision.")
print("")

df = pd.DataFrame({"score_xgb": score_xgb})
# df["score_xgb (logistic)"] = 1./(1. + np.exp(-1 * (df["score_xgb"] + 1.0)))
# df["score_xgb (gradboost)"] = 2./(1. + np.exp(-2 * (df["score_xgb"] + 1.0))) - 1.
df["score_gbr"] = score_gbr
# df["score_gbr_gradboost"] = score_gbr_gradboost

print(df)

# import matplotlib.pyplot as plt

# plt.scatter(df["score_xgb"].values, df["score_gbr"].values)
# plt.show()
