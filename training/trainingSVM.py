
import pandas as pd
from sklearn import model_selection
from sklearn import svm
from sklearn import metrics
df = pd.read_csv('LOGDATA.log', names=["label","band","data"])
print(df)
%matplotlib inline
import matplotlib.pyplot as plt
plt.scatter(df["band"],df["data"],c=df["label"])
feature = df[["band","data"]]
target = df["label"]
feature_train, feature_test, target_train, target_test = model_selection.train_test_split(feature, target, test_size=0.2)

#Train SVM
model = svm.SVC(kernel="linear",verbose=1)
model.fit(feature_train, target_train)

#calcurate training model accuracy
pred_train = model.predict(feature_train)
metrics.accuracy_score(target_train, pred_train)

#calcurate test model accuracy
pred_test = model.predict(feature_test)
metrics.accuracy_score(target_test, pred_test)

# Save Model
filename = 'finalized_model.sav'
pickle.dump(model, open(filename, 'wb'))
