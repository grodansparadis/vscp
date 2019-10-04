# FILE: receive.py
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
# VSCP Event to/from node
# -----------------------
# 0x10  - DLE
# 0x02  - STX
# 0x01  - frametype=command   crc
# 0xxx  - channel             crc
# 0xxx  - seq no              crc
# 0xxx  - Size of payload     crc
# 0xxx  - Size o payload      crc
# 0xxx  - payload             crc
# See https://grodansparadis.gitbooks.io/the-vscp-specification/vscp_over_a_serial_channel_rs-232.html#frame-type1---vscp-event
# 0xxx  - crc
# 0x10  - DLE
# 0x03  - ETX
#
# CANAL message to/from node
# --------------------------
# 0x10  - DLE
# 0x02  - STX
# 0x02  - frametype=command   crc
# 0xxx  - channel             crc
# 0xxx  - seq no              crc
# 0xxx  - Size of payload     crc
# 0xxx  - Size o payload      crc
# 0xxx  - payload             crc
# See https://grodansparadis.gitbooks.io/the-vscp-specification/vscp_over_a_serial_channel_rs-232.html#frame-type2---canal-message
# 0xxx  - crc
# 0x10  - DLE
# 0x03  - ETX

import sys
import time
import serial   # https://pythonhosted.org/pyserial/pyserial.html
import crc8     # https://pypi.org/project/crc8/

inbuf = bytearray()

can4vscp_port = "/dev/ttyUSB0"
can4vscp_baudrate = 115200

print("Receiving VSCP serial frames")

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
hash.update(b'\xff\x00\x00\x00\x01\x04')
print('CRC: ' + hash.hexdigest())

ser.isOpen()

bDle = False
bStx = False

while 1:
	while ser.inWaiting() > 0:
		b = ser.read(1)
		print(hex(b[0]) + ' ', end='', flush=True)

		if bDle :
			if (b[0] == 2) :   # stx
				print('| ', end='', flush=True)
				bDle = False
			elif (b[0] == 3) : # dle
				print('|')
				bDle = False
			else:
				bDle = False
		elif b[0] == 0x10 :
			bDle = True
		else:
			bDle = False

ser.close()

print('OK')
