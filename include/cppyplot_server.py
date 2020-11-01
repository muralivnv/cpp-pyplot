import zmq
import sys

from threading import Thread
import struct

context = zmq.Context()
socket = context.socket(zmq.SUB)
if (len(sys.argv) > 1):
    socket.connect(sys.argv[1])
else:
    socket.connect("tcp://127.0.0.1:5555")
socket.setsockopt(zmq.LINGER, 0)
socket.setsockopt_string(zmq.SUBSCRIBE, "")

############
## function to receive all publisher messages
############
import queue
msg_queue   = queue.Queue()
kill_thread = False

def subscriber():
    global socket, msg_queue
    while (not kill_thread):
        if (socket.poll(50, zmq.POLLIN)):
            zmq_message = socket.recv()
            msg_queue.put(zmq_message)

subscriber_thread = Thread(target=subscriber)
subscriber_thread.start()
print("[INFO] Started subscriber thread ")

lib_sym = {}

## Import numpy and register the object in the symbol table
import numpy as np
lib_sym['np'] = np

## Import matplotlib and register plot object in the symbol table
import matplotlib.pyplot as plt
lib_sym['plt'] = plt

# from matplotlib.animation import FuncAnimation
# lib_sym['FuncAnimation'] = FuncAnimation

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


from asteval import Interpreter, make_symbol_table
aeval          = Interpreter()
aeval.symtable = make_symbol_table(use_numpy=True, **lib_sym)
plot_cmd  = None
plot_data = {}

SYM_IDX   = 1
TYPE_IDX  = 2
LEN_IDX   = 3
SHAPE_IDX = 4

print("[INFO] Plotting server initialized ...")

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

def handle_payload(data, data_type, data_len, data_shape):
    if ((data_type == 'c') or (data_type == 'b') or (data_type == 'B')):
        data_converted = struct.unpack("="+(data_type*data_len), data)
        return (b''.join(data_converted)).decode("utf-8")
    else:
        if data_shape[0] > 0:
            return np.ndarray(data_shape, dtype="="+data_type, buffer=data)
        else:
            return (struct.unpack("="+data_type, data))[0]

try:
    while(True):
        zmq_message = None
        if (not msg_queue.empty()):
            zmq_message = msg_queue.get()
            msg_queue.task_done()
        else:
            continue
        
        if (zmq_message[0:4] == b"data"):
            data_info     = zmq_message.decode("utf-8").split('|')
            # 0: data, 1: var_name, 2: var_type, 3: n_elems, 4: array_shape
            data_type     = data_info[TYPE_IDX]
            data_len      = int(data_info[LEN_IDX])
            data_shape    = parse_shape(data_info[SHAPE_IDX])
            data_payload  = msg_queue.get()
            plot_data[data_info[SYM_IDX]] = handle_payload(data_payload, data_type, data_len, data_shape)
            msg_queue.task_done()
        elif(zmq_message[0:8] == b"finalize"):
            aeval.symtable = {**aeval.symtable, **plot_data}
            aeval.eval(plot_cmd)

            # Some error happened pause execution by creating sample matplotlib windows
            if aeval.error_msg != None:
                aeval.error_msg = None
                plt.figure(figsize=(6,5))
                plt.title("Exception from ASTEVAL, check stdout", fontsize=14)
                plt.show()
            plot_cmd  = None
            plot_data = {}
        elif(zmq_message == b"exit"):
            print("[INFO] Received exit message, exiting")
            kill_thread = True
            subscriber_thread.join()
            sys.exit(0)
        else:
            # print("[INFO] Executing ... \n")
            plot_cmd = zmq_message.decode("utf-8")
            # print(plot_cmd, '\n')
except KeyboardInterrupt as e:
    print("[Error] Received keyboardInterrupt, killing subscriber thread ")
    kill_thread = True
    subscriber_thread.join()
    sys.exit(e)
