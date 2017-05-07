#!/usr/bin/env python3

import inputs
import serial
import time
import random
from struct import pack

code2axis = {'ABS_X': 'yaw', 'ABS_Y': 'pitch', 'ABS_RX': 'roll'}
axis2dim = {'yaw': 0, 'pitch': 1, 'roll': 2, 'tweeze': 3}
code2dim = {k: axis2dim[code2axis[k]] for k in code2axis.keys()}

invert_axis = {'pitch': False, 'yaw': False, 'roll': False}

port = '/dev/ttyACM2'
baud = 9600
zero_below = 130
min_interval = 0.01

with serial.Serial(port=port, baudrate=baud, timeout=0, write_timeout=0.25) as ard:
    while True:
        es = list(inputs.get_gamepad())
        random.shuffle(es)
        for e in es:
            if e.code in code2dim:
                # states seem to be encoded in multiples of 256 (possibly excluding 0)
                print(e.code)
                print(e.state)

                state = e.state
                if abs(state) < zero_below:
                    state = 0

                # < = Little-Endian (Arduino)
                # B = unsigned char (1 byte)
                # h = signed short  (2 bytes, the size of arduino ints)
                to_send = pack('<Bh', code2dim[e.code], state)
                try:
                    written = ard.write(to_send)
                except serial.serialutil.SerialTimeoutException:
                    print('write timeout')
                    pass


