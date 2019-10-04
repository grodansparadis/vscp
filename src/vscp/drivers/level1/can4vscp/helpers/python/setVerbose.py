import sys
import time
import serial   # https://pythonhosted.org/pyserial/pyserial.html
import crc8     # https://pypi.org/project/crc8/

can4vscp_port = "/dev/ttyUSB0"
can4vscp_baudrate = 115200

print("Sending 'close' frame")

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

#ser.open()
ser.isOpen()

# Send the can4vscp close frame
ser.write(b'\x10\x02\xff\x00\x00\x00\x01\x04')
ser.write(hash.digest())
ser.write(b'x10\x03')

ser.close()

print('OK')
