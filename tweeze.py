#!/usr/bin/env python3

import inputs
import serial
from struct import pack

code2axis = {'ABS_X': 'yaw', 'ABS_Y': 'pitch', 'ABS_RX': 'roll'}
axis2dim = {'yaw': 0, 'pitch': 1, 'roll': 2, 'tweeze': 3}
code2dim = {k: axis2dim[code2axis[k]] for k in code2axis.keys()}

invert_axis = {'pitch': False, 'yaw': False, 'roll': False}

#port = '/dev/ttyACM1'
#baud = 9600
#ard = serial.Serial(port, baud)

while True:
    es = inputs.get_gamepad()
    for e in es:
        print(e.ev_type, e.code, e.state)
        if e.code in code2dim:
            # < = Little-Endian (Arduino)
            # B = unsigned char (1 byte)
            # h = signed short  (2 bytes, the size of arduino ints)
            to_send = pack('<Bh', code2dim[e.code], e.state)
            #ard.write(to_send)

            print(code2axis[e.code])
            print(code2dim[e.code])
            print(e.state)
            print('')
