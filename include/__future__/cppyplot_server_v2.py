from argparse import ArgumentParser

cmd_parser = ArgumentParser(description="Cppyplot helper file to handle plot commands")
cmd_parser.add_argument("--sub_addr", type=str, default="tcp://127.0.0.1:5555", help="address for the subscriber to connect to"    )
cmd_parser.add_argument("--pub_addr", type=str, default="tcp://127.0.0.1:5556", help="address for the publisher to publish data to")
cmd_parser.add_argument("--custom_import_file_path", type=str, default="", help="custom py file which contains imports to use")
cmd_args = cmd_parser.parse_args()

import zmq
context = zmq.Context()
socket_sub = context.socket(zmq.SUB)
socket_pub = context.socket(zmq.PUB)
socket_sub.connect(cmd_args.sub_addr)
socket_pub.bind(cmd_args.pub_addr)
socket_sub.setsockopt(zmq.LINGER, 0)
socket_sub.setsockopt_string(zmq.SUBSCRIBE, "")

def subscriber():
    global socket_sub, msg_queue
    while (not kill_thread):
        if (socket_sub.poll(50, zmq.POLLIN)):
            zmq_message = socket_sub.recv()
            msg_queue.put(zmq_message)

def handle_custom_import_file(filename:str) -> dict:
  from sys import path
  from importlib import import_module
  from os.path import dirname, basename, exists
  
  custom_module_sym = {}

  if (exists(filename)):
    path.append(dirname(filename))
    file_base = basename(filename)
    file_base = file_base.split('.')[0]
    custom_module = import_module(file_base)
    for key in dir(custom_module):
      if not key.startswith("__"):
        custom_module_sym[key] = getattr(custom_module, key)
  else:
    print(f"{filename} does not exist")
    import matplotlib.pyplot as plt
    plt.figure(figsize=(6,5))
    plt.title("Exception from ASTEVAL, check stdout", fontsize=14)
    plt.show()
  
  return custom_module_sym

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

def create_payload(data_type, data_len, data_shape):
  payload = np.ndarray(data_shape, dtype="="+data_type)
  return payload

from threading import Thread
import queue
msg_queue   = queue.Queue()
kill_thread = False

subscriber_thread = Thread(target=subscriber)
subscriber_thread.start()
print("[INFO] Started subscriber thread ")

lib_sym = {}
if (cmd_args.custom_import_file_path != ""):
  custom_lib_sym = handle_custom_import_file(cmd_args.custom_import_file_path)
  lib_sym = {**lib_sym, **custom_lib_sym}

import struct
from asteval import Interpreter, make_symbol_table
aeval          = Interpreter()
aeval.symtable = make_symbol_table(use_numpy=True, **lib_sym)
plot_cmd       = None
plot_data      = {}
vars_to_send_back = []

SYM_IDX   = 1
TYPE_IDX  = 2
LEN_IDX   = 3
SHAPE_IDX = 4
CMD_TYPE  = 5

print("[INFO] Plotting server initialized ...")

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
            # 0: data, 1: var_name, 2: var_type, 3: n_elems, 4: array_shape, 5: cmd_type (use/use&return)
            data_type     = data_info[TYPE_IDX]
            data_len      = int(data_info[LEN_IDX])
            data_shape    = parse_shape(data_info[SHAPE_IDX])
            cmd_type      = data_info[CMD_TYPE]
            if (cmd_type == "send_back"):
              plot_data[data_info[SYM_IDX]] = create_payload(data_type, data_len, data_shape)
              vars_to_send_back.append(data_info[SYM_IDX])
            else:
              data_payload  = msg_queue.get()
              plot_data[data_info[SYM_IDX]] = handle_payload(data_payload, data_type, data_len, data_shape)
              msg_queue.task_done()
        elif(zmq_message[0:8] == b"finalize"):
            aeval.symtable = {**aeval.symtable, **plot_data}
            aeval.eval(plot_cmd)

            # Some error happened pause execution by creating sample matplotlib windows
            if aeval.error_msg != None:
              aeval.error_msg = None
              import matplotlib.pyplot as plt
              plt.figure(figsize=(6,5))
              plt.title("Exception from ASTEVAL, check stdout", fontsize=14)
              plt.show()
            elif (any(vars_to_send_back)):
              for key in vars_to_send_back:
                socket_pub.send_string(key)
                socket_pub.send(aeval.symtable[key])
              vars_to_send_back = []

            plot_cmd  = None
            plot_data = {}
        elif(zmq_message == b"exit"):
            print("[INFO] Received exit message, exiting")
            kill_thread = True
            subscriber_thread.join()
            from sys import exit
            exit(0)
        else:
            plot_cmd = zmq_message.decode("utf-8")
except KeyboardInterrupt as e:
    print("[Error] Received keyboardInterrupt, killing subscriber thread ")
    kill_thread = True
    subscriber_thread.join()
    from sys import exit
    exit(e)
