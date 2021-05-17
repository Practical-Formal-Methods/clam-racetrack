import tensorflow as tf
import numpy as np
import os
import csv
import sys

model_name = sys.argv[1]

#snapshot parameters
MODEL_PREF = "model"

INPUT_SIZE = 14
HIDDEN_SIZE = 64
OUTPUT_SIZE = 9

names = [
    'fc1',
    'fc2',
    'fc3'
]

params = [
    'kernel',
    'bias'
]
def get_reader(ckptpath):
    print("reading {}/variables/variables ".format(ckptpath))
    reader = tf.train.NewCheckpointReader(ckptpath+"/variables/variables")
    all_variables = reader.get_variable_to_shape_map()
    print(all_variables)
    #for n in all_variables:
    #    print(n)
    #    t = reader.get_tensor(n)
    #    print(t.shape)
    #    print(t[0])
    return reader

def dump_rlv_model(ckptpath,output=None):
    layer_names = [
        'in',
        'relu1',
        'relu2',
        'Accuracy'
    ]
    neuron_num = [
        INPUT_SIZE,
        HIDDEN_SIZE,
        HIDDEN_SIZE,
        OUTPUT_SIZE
    ]
    neuron_type = [
        'Input',
        'ReLU',
        'ReLU',
        'Linear'
    ]
    reader = get_reader(ckptpath)
    layer = 0
    print("# Layer 0 14 Input in", file=output)
    for i in range(neuron_num[0]):
        print("{} in_{}".format(neuron_type[0],i), file=output)
    for n in names:
        layer += 1
        print(
            "# Layer {layer_index} {number} {type} {name}".format(
                layer_index = layer,
                number = neuron_num[layer],
                type = neuron_type[layer],
                name = layer_names[layer]
            ),
            file=output
        )# TODO
        W = reader.get_tensor(n + ".weight_turned_var")
        b = reader.get_tensor(n + ".bias_turned_var")
        for i in range(neuron_num[layer]):
            print("{} {}_{} {}".format(
                    neuron_type[layer], 
                    layer_names[layer],
                    i,
                    b[i]), 
                end='',
                file=output
            )
            for j in range(neuron_num[layer-1]):
                print(" {} {}_{}".format(
                        W[i][j],
                        layer_names[layer-1],
                        j),
                    end='',
                    file=output
                )
            print(file=output)

if __name__ == "__main__":
    model_name=sys.argv[1]
    rlv_file_name=model_name+".rlv"
    with open('./'+model_name+'/'+rlv_file_name, 'w') as f:
        dump_rlv_model(model_name,f)
