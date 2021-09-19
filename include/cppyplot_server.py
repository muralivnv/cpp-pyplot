### Custom Imports ###
lib_sym = {}

## Import numpy and register the object in the symbol table
import numpy as np
lib_sym['np'] = np

## Import matplotlib and register plot object in the symbol table
import matplotlib.pyplot as plt
import matplotlib.cm  as cm
lib_sym['plt'] = plt
lib_sym['cm'] = cm

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

# Import plotly
# import plotly.io as pio
# # pio.renderers.default = "browser" # comment this when using Dash for rendering
# import plotly.graph_objects as go
# from plotly.subplots import make_subplots
# lib_sym['go']            = go
# lib_sym['make_subplots'] = make_subplots
# lib_sym['pio']           = pio

# Import dash
# import dash
# import dash_core_components as dcc
# import dash_html_components as html
# lib_sym['dash'] = dash
# lib_sym['dcc'] = dcc
# lib_sym['html'] = html

#### required imports ####
import zmq
import sys
from threading import Thread
from struct import unpack
from queue import Queue
from asteval import Interpreter, make_symbol_table

#### Globals #####
recv_msgs   = Queue()
parsed_msgs = Queue()
kill_thread = False

aeval = Interpreter()
aeval.symtable = make_symbol_table(use_numpy=True, **lib_sym, no_print=False)

# indices for data access
SYM_IDX   = 1
TYPE_IDX  = 2
LEN_IDX   = 3
SHAPE_IDX = 4

#### utility functions ####
def subscriber(addr):
    global recv_msgs
    context = zmq.Context()
    socket = context.socket(zmq.SUB)
    socket.connect(addr)
    socket.setsockopt(zmq.LINGER, 0)
    socket.setsockopt_string(zmq.SUBSCRIBE, "")
    
    print("[INFO] started subscriber thread ")
    print(f"[INFO] listening to {addr}")
    while (not kill_thread):
        if (socket.poll(50, zmq.POLLIN)):
            zmq_message = socket.recv()
            recv_msgs.put(zmq_message)

def parse_shape(shape_str:str)->list:
    shape = []
    axis_size = ""
    for char in shape_str:
        if (char.isdigit()):
            axis_size = axis_size + char
        elif(char==',' or char==')'):
            if (axis_size != ""):
                shape.append(int(axis_size))
                axis_size = ""
    return shape

def handle_payload(data, data_type, data_len, data_shape, _unpack=unpack):
    if ((data_type == 'c') or (data_type == 'b') or (data_type == 'B')):
        data_converted = _unpack("="+(data_type*data_len), data)
        return (b''.join(data_converted)).decode("utf-8")
    else:
        if data_shape[0] > 0:
            return np.ndarray(data_shape, dtype="="+data_type, buffer=data)
        else:
            return (_unpack("="+data_type, data))[0]

def update_data(header, data, plot_data:dict)->dict:
    data_info     = header.decode("utf-8").split('|')

    # 0: data, 1: var_name, 2: var_type, 3: n_elems, 4: array_shape
    data_type     = data_info[TYPE_IDX]
    data_len      = int(data_info[LEN_IDX])
    data_shape    = parse_shape(data_info[SHAPE_IDX])
    plot_data[data_info[SYM_IDX]] = handle_payload(data, data_type, data_len, data_shape)

    return plot_data

def update_cmd(zmq_message)->str:
    return zmq_message.decode("utf-8")

def parse_msgs():
    global parsed_msgs, recv_msgs
    plot_cmd  = None
    plot_data = {}
    while (not kill_thread):
        zmq_message = None
        if (not recv_msgs.empty()):
            zmq_message = recv_msgs.get()
            recv_msgs.task_done()
        else:
            continue
        
        if (zmq_message[0:4] == b"data"):
            header = zmq_message
            data   = recv_msgs.get()
            recv_msgs.task_done()
            plot_data = update_data(header, data, plot_data)

        elif (zmq_message[0:8] == b"finalize"):
            parsed_msgs.put(("plot", plot_cmd, plot_data, ))
            plot_cmd = None
            plot_data = {}
        elif (zmq_message == b"exit"):
            parsed_msgs.put(("exit", 0,))
        else:
            plot_cmd = update_cmd(zmq_message)

def plot_handler(plot_cmd:str, plot_data:dict)->None:
    global aeval
    print("[INFO] plotting ...")
    aeval.symtable = {**aeval.symtable, **plot_data}
    aeval.eval(plot_cmd)

    if (aeval.error_msg != None):
        aeval.error_msg = None
        plt.figure(figsize=(6,5))
        plt.title("Exception from ASTEVAL, check stdout", fontsize=14)
        plt.show()
    print("[INFO] done")

def exit_handler(exit_code):
    global kill_thread
    kill_thread = True
    print("[INFO] requested to shutdown, goodbye!")

def run_main():
    global parsed_msgs
    cmd_handler = {}
    cmd_handler["plot"] = plot_handler
    cmd_handler["exit"] = exit_handler

    print(f"[INFO] plotting server initialized")
    try:
        while(not kill_thread):
            if (not parsed_msgs.empty()):
                msg = parsed_msgs.get_nowait()
                parsed_msgs.task_done()
                func = cmd_handler[msg[0]]
                func(*(msg[1:]))
    except KeyboardInterrupt:
        print("[Warning] received keyboardInterrupt, killing plotting server")
        exit_handler(0)

#### main ####
if __name__ == '__main__':
    addr = "tcp://127.0.0.1:5555"
    if (len(sys.argv) > 1):
        addr = sys.argv[1]
    
    sub_thread = Thread(target=subscriber, args=[addr])
    parser_thread = Thread(target=parse_msgs)

    sub_thread.start()
    parser_thread.start()
    
    run_main()

    sub_thread.join()
    parser_thread.join()
