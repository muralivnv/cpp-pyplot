import zmq
import sys

context = zmq.Context()
socket = context.socket(zmq.SUB)
if (len(sys.argv) > 1):
    socket.connect(sys.argv[1])
else:
    socket.connect("tcp://127.0.0.1:5555")
socket.setsockopt(zmq.LINGER, 0)
socket.setsockopt_string(zmq.SUBSCRIBE, "")

lib_sym = {}

import struct

## Import numpy and register the object in the symbol table
import numpy as np
lib_sym['np'] = np

## Import matplotlib and register plot object in the symbol table
import matplotlib.pyplot as plt
lib_sym['plt'] = plt

## Import seaborn and register seaborn object in the symbol table
# import seaborn as sns
# lib_sym['sns'] = sns

## Import pandas and register pandas object in the symbol table
# import pandas as pd
# lib_sym['pd'] = pd

# Import bokeh and register bokeh objects in the symbol table
# from bokeh.layouts import column, row
# lib_sym['column'] = column
# lib_sym['row']    = row

# from bokeh.plotting import ColumnDataSource, figure, output_file, show
# lib_sym['ColumnDataSource'] = ColumnDataSource
# lib_sym['figure']           = figure
# lib_sym['output_file']      = output_file
# lib_sym['show']             = show

from asteval import Interpreter

print("Started Plotting Client .... ")

plot_cmd  = None
plot_data = {}

def parse_shape(shape_str:str)->tuple:
    shape = []
    axis_size = ""
    for char in shape_str:
        if (char.isdigit()):
            axis_size = axis_size + char
        elif(char==',' or char==')'):
            if (axis_size != ""):
                shape.append(int(axis_size))
                axis_size = ""
    
    return tuple(shape)

while(True):
    zmq_message = socket.recv()
    if (zmq_message[0:4] == b"data"):
        data_info     = zmq_message.decode("utf-8").split('|')
        
        # 0: data, 1: var_name, 2: var_type, 3: n_elems, 4: array_shape
        data_type     = data_info[2]
        data_len      = int(data_info[3])
        data_shape    = parse_shape(data_info[4])
        data_payload  = socket.recv()
        data_payload  = np.array((struct.unpack("="+(data_type*data_len), data_payload)))
        plot_data[data_info[1]] = np.reshape(data_payload, data_shape)
    elif(zmq_message[0:8] == b"finalize"):
        sym_table = {**lib_sym, **plot_data}
        aeval = Interpreter(usersyms=sym_table)
        aeval.eval(plot_cmd)

        # Some error happened pause execution by creating sample matplotlib windows
        if aeval.error_msg != None:
            plt.figure(figsize=(6,5))
            plt.title("Exception from ASTEVAL, check stdout", fontsize=14)
            plt.show()
        plot_cmd  = None
        plot_data = {}
    elif(zmq_message == b"exit"):
        sys.exit(0)
    else:
        plot_cmd = zmq_message.decode("utf-8")