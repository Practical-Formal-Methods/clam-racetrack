import tensorflow as tf
import numpy as np
import os
import csv
import sys

# Require eran
sys.path.append("../eran/tf_verify")
sys.path.insert(0, '../eran/tf_verify/../ELINA/python_interface/')
from eran import ERAN
from read_net_file import *

#snapshot parameters
MODEL_PREF = "model"

INPUT_SIZE = 14
HIDDEN_SIZE = 64
OUTPUT_SIZE = 9

export_path = './models_20191121'
names = [
    'fc1',
    'fc2',
    'fc3'
]

#names = [
#    'main/dense',
#    'main/dense_1',
#    'main/output'
#]
#params = [
#    'kernel',
#    'bias'
#]
params = [
    'weight_turned_var',
    'bias_turned_var'
]

def get_reader(ckptpath):
    print("reading {}/variables/variables ".format(ckptpath))
    reader = tf.train.NewCheckpointReader(ckptpath+"/variables/variables")
    # all_variables = reader.get_variable_to_shape_map()
    # print(all_variables)
    # for n in all_variables:
    #     print(n)
    #     t = reader.get_tensor(n)
    #     print(t.shape)
    #     # print(t[0])
    return reader


def dump_eran_model(ckptpath,output=None):
    reader = get_reader(ckptpath)
    for name in names:
        print("ReLU",file=output)
        for param in params:
            tensor_name = name + "." + param
            print((reader.get_tensor(tensor_name)).tolist(),file=output)

def verify(eran_model_path,input,epsilon):
    model, is_conv, means, stds = read_net(eran_model_path, 14, False)
    eran = ERAN(model)

    specLB = np.clip(input - epsilon, 0, 1)
    specUB = np.clip(input + epsilon, 0, 1)
    for i in range(0,2):
        specLB[i] = input[i]
        specUB[i] = input[i]
    for i in range(2,4):
        specLB[i] = np.clip(input[i] - epsilon,-1,1)
        specUB[i] = np.clip(input[i] + epsilon,-1,1)
    label, nn, _, _ = eran.analyze_box(specLB, specUB, 'deepzono', 1, 1)
    print("lable = {}".format(label))
    return label >= 0

def find_delta(eran_model_path,input):
    l = 0.0
    r = 1.0
    while (r-l >= 0.0001):
        mid = ( l + r ) / 2
        print("l = {}, mid = {}, r = {}".format(l,mid,r))
        if verify(eran_model_path,input,mid):
            l = mid
        else:
            r = mid
    return l

def verify_snapshots(TIMESTAMP,TYPE,TESTS):
    SNAP_PATH = os.path.join(MODEL_PREF,TIMESTAMP)
    SNAPS = [snap for snap in os.listdir(SNAP_PATH) if os.path.isdir(os.path.join(SNAP_PATH,snap))]
    SNAPS.sort()
    print("=========================")
    print("Verify model:{} on dataset:{}".format(SNAP_PATH,TESTS))
    print("sanpshots:",end="")
    print(SNAPS)
    print("=========================")

    summary_csv_file = open(os.path.join(SNAP_PATH,"robust.csv"),"w")
    summary = csv.writer(summary_csv_file)
    summary.writerow(["SNAP","Point","delta"])
    for snap in SNAPS:
        model_path = os.path.join(SNAP_PATH,snap)
        eran_model_path = os.path.join(model_path,"eran.tf")
        output = open(eran_model_path, "w")
        dump_eran_model(model_path,output)
        output.close()

        tests_csv_file = open(TESTS, 'r')
        tests = csv.reader(tests_csv_file, delimiter=',')
        for idx,test in enumerate(tests):
            input = np.float64(test)
            delta = find_delta(eran_model_path,input)
            summary.writerow([snap,idx,delta])
    summary_csv_file.close()

if __name__ == "__main__":
    #model_name="2019-12-01 22:44:53.403846"
    #tests="test_100.csv"
    #verify_snapshots(model_name,"ERAN",tests)
    model_name=sys.argv[1]
    basename = os.path.splitext(model_name)[0]
    tf_filename = "./"+basename+".tf"
    with open(tf_filename, 'w') as f:
        dump_eran_model(model_name, f)
