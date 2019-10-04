# FILE: commandSetVerboseMode.py
#
# This file is part of the VSCP (http://www.vscp.org)
#
# The MIT License (MIT)
#
# Copyright (C) 2000-2019 Ake Hedman, Grodans Paradis AB
# <info@grodansparadis.com>
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
#

# Close the channel for a serial device that use the VSCP serial protocol.
# Usage: python3 commandClose /dev/ttyUSB1 115200
# Default port is '/dev/ttyUSB0'
# Default baudrate is '115200'
#
# Change driver mode frame
# ------------------------
# 0x10  - DLE
# 0x02  - STX
# 0xff  - frametype=command
# 0x00  - channel
# 0x00  - seq no
# 0x00  - Size of payload
# 0x02  - Size o payload
# 0x01  - set mode
# 0x00  - verbose
# 0x87  - crc
# 0x10  - DLE
# 0x03  - ETX

import sys
import time
import serial   # https://pythonhosted.org/pyserial/pyserial.html
import crc8     # https://pypi.org/project/crc8/

can4vscp_port = "/dev/ttyUSB0"
can4vscp_baudrate = 115200

print("Sending 'set verbode mode' frame")

if ( len(sys.argv) >= 2 ):
    can4vscp_port = sys.argv[1]
    print('Using serial port: ' + can4vscp_port)

if ( len(sys.argv) >= 3 ):
    can4vscp_baudrate = int(sys.argv[2])
    print('Using baudrate: %d' % can4vscp_baudrate)

# configure the serial connections (the parameters differs on the device you are connecting to)
ser = serial.Serial(
	port = can4vscp_port,
	baudrate = can4vscp_baudrate,
	parity = serial.PARITY_NONE,
	stopbits = serial.STOPBITS_ONE,
	bytesize = serial.EIGHTBITS
)

hash = crc8.crc8()
hash.update(b'\xff\x00\x00\x00\x02\x01\x00')
print('CRC: ' + hash.hexdigest())

#ser.open()
ser.isOpen()

# Send the can4vscp close frame
ser.write(b'\x10\x02\xff\x00\x00\x00\x02\x01\x00')
ser.write(hash.digest())
ser.write(b'x10\x03')

ser.close()

print('OK')
