#!/usr/bin/env python
#
# Send/receive UDP multicast packets.
# Requires that your OS kernel supports IP multicast.
#
# Usage:
#   mcast -s (sender, IPv4)
#   mcast -s -6 (sender, IPv6)
#   mcast    (receivers, IPv4)
#   mcast  -6  (receivers, IPv6)

import time
import struct
import socket
import sys
import datetime
# pip install pycrc
from PyCRC.CRCCCITT import CRCCCITT

MCAST_PORT = 44444
MCAST_GROUP = '224.0.23.158'
MCAST_TTL = 1   # Increase to reach other networks

VSCP_MULTICAST_PACKET0_HEADER_LENGTH = 35
VSCP_MULTICAST_PACKET0_POS_PKTTYPE = 0
VSCP_MULTICAST_PACKET0_POS_HEAD = 1
VSCP_MULTICAST_PACKET0_POS_HEAD_MSB = 1
VSCP_MULTICAST_PACKET0_POS_HEAD_LSB = 2
VSCP_MULTICAST_PACKET0_POS_TIMESTAMP = 3
VSCP_MULTICAST_PACKET0_POS_YEAR = 7
VSCP_MULTICAST_PACKET0_POS_YEAR_MSB = 7
VSCP_MULTICAST_PACKET0_POS_YEAR_LSB = 8
VSCP_MULTICAST_PACKET0_POS_MONTH = 9
VSCP_MULTICAST_PACKET0_POS_DAY = 10
VSCP_MULTICAST_PACKET0_POS_HOUR = 11
VSCP_MULTICAST_PACKET0_POS_MINUTE = 12
VSCP_MULTICAST_PACKET0_POS_SECOND = 13
VSCP_MULTICAST_PACKET0_POS_VSCP_CLASS = 14
VSCP_MULTICAST_PACKET0_POS_VSCP_CLASS_MSB = 14
VSCP_MULTICAST_PACKET0_POS_VSCP_CLASS_LSB = 15
VSCP_MULTICAST_PACKET0_POS_VSCP_TYPE = 16
VSCP_MULTICAST_PACKET0_POS_VSCP_TYPE_MSB = 16
VSCP_MULTICAST_PACKET0_POS_VSCP_TYPE_LSB = 17
VSCP_MULTICAST_PACKET0_POS_VSCP_GUID = 18
VSCP_MULTICAST_PACKET0_POS_VSCP_SIZE = 34
VSCP_MULTICAST_PACKET0_POS_VSCP_SIZE_MSB = 34
VSCP_MULTICAST_PACKET0_POS_VSCP_SIZE_LSB = 35
VSCP_MULTICAST_PACKET0_POS_VSCP_DATA = 36

################################################################################
# main
#

def main():
    print("mcast\n")
#    group = MYGROUP_6 if "-6" in sys.argv[1:] else MYGROUP_4

#    if "-s" in sys.argv[1:]:
    sender( MCAST_GROUP )
#    else:
#        receiver( group )

################################################################################
# Create a VSCP frame
#

def makeVscpFrame( encryption ):

    frame = bytearray( 1 + VSCP_MULTICAST_PACKET0_HEADER_LENGTH + 512 + 2 )

    # Frame type, Type 0, unencrypted
    frame[ VSCP_MULTICAST_PACKET0_POS_PKTTYPE ] = 0

    # Head
    frame[ VSCP_MULTICAST_PACKET0_POS_HEAD_MSB ] = 0
    frame[ VSCP_MULTICAST_PACKET0_POS_HEAD_LSB ] = 0

    # Timestamp
    frame[ VSCP_MULTICAST_PACKET0_POS_TIMESTAMP ] = 0
    frame[ VSCP_MULTICAST_PACKET0_POS_TIMESTAMP + 1 ] = 0
    frame[ VSCP_MULTICAST_PACKET0_POS_TIMESTAMP + 2 ] = 0
    frame[ VSCP_MULTICAST_PACKET0_POS_TIMESTAMP + 3 ] = 0

    # UTC time
    dt = datetime.datetime.utcnow()

    # Date / time block 1956-11-02 04:23:52 GMT
    frame[ VSCP_MULTICAST_PACKET0_POS_YEAR_MSB ] = (dt.year >> 8) & 0xff
    frame[ VSCP_MULTICAST_PACKET0_POS_YEAR_LSB ] = dt.year& 0xff
    frame[ VSCP_MULTICAST_PACKET0_POS_MONTH ] = dt.month
    frame[ VSCP_MULTICAST_PACKET0_POS_DAY ] = dt.day
    frame[ VSCP_MULTICAST_PACKET0_POS_HOUR ] = dt.hour
    frame[ VSCP_MULTICAST_PACKET0_POS_MINUTE ] = dt.minute
    frame[ VSCP_MULTICAST_PACKET0_POS_SECOND ] = dt.second

    # Class = 1040 Measurement String
    frame[ VSCP_MULTICAST_PACKET0_POS_VSCP_CLASS_MSB ] = 0x04
    frame[ VSCP_MULTICAST_PACKET0_POS_VSCP_CLASS_LSB ] = 0x10

    # Type = Temperature = 6
    frame[ VSCP_MULTICAST_PACKET0_POS_VSCP_TYPE_MSB ] = 0x00
    frame[ VSCP_MULTICAST_PACKET0_POS_VSCP_TYPE_LSB ] = 0x06

    # GUID - dummy
    frame[ VSCP_MULTICAST_PACKET0_POS_VSCP_GUID ] = 0x00
    frame[ VSCP_MULTICAST_PACKET0_POS_VSCP_GUID + 1 ] = 0x01
    frame[ VSCP_MULTICAST_PACKET0_POS_VSCP_GUID + 2 ] = 0x02
    frame[ VSCP_MULTICAST_PACKET0_POS_VSCP_GUID + 3 ] = 0x03
    frame[ VSCP_MULTICAST_PACKET0_POS_VSCP_GUID + 4 ] = 0x04
    frame[ VSCP_MULTICAST_PACKET0_POS_VSCP_GUID + 5 ] = 0x05
    frame[ VSCP_MULTICAST_PACKET0_POS_VSCP_GUID + 6 ] = 0x06
    frame[ VSCP_MULTICAST_PACKET0_POS_VSCP_GUID + 7 ] = 0x07
    frame[ VSCP_MULTICAST_PACKET0_POS_VSCP_GUID + 8 ] = 0x08
    frame[ VSCP_MULTICAST_PACKET0_POS_VSCP_GUID + 9 ] = 0x09
    frame[ VSCP_MULTICAST_PACKET0_POS_VSCP_GUID + 10 ] = 0x0A
    frame[ VSCP_MULTICAST_PACKET0_POS_VSCP_GUID + 11 ] = 0x0B
    frame[ VSCP_MULTICAST_PACKET0_POS_VSCP_GUID + 12 ] = 0x0C
    frame[ VSCP_MULTICAST_PACKET0_POS_VSCP_GUID + 13 ] = 0x0D
    frame[ VSCP_MULTICAST_PACKET0_POS_VSCP_GUID + 14 ] = 0x0E
    frame[ VSCP_MULTICAST_PACKET0_POS_VSCP_GUID + 15 ] = 0x0F

    # Size
    frame[ VSCP_MULTICAST_PACKET0_POS_VSCP_SIZE_MSB ] = 0
    frame[ VSCP_MULTICAST_PACKET0_POS_VSCP_SIZE_LSB ] = 13

    # Data  Sensor index=2, Zone=1, sunzone2", Unit=1 (Celsius)
    # Temperature = 123.45678
    frame[ VSCP_MULTICAST_PACKET0_POS_VSCP_DATA ] = 0x02        # Sensor idx = 2
    frame[ VSCP_MULTICAST_PACKET0_POS_VSCP_DATA + 1 ] = 0x01    # Zone = 1
    frame[ VSCP_MULTICAST_PACKET0_POS_VSCP_DATA + 2 ] = 0x02    # Subzone = 2
    frame[ VSCP_MULTICAST_PACKET0_POS_VSCP_DATA + 3 ] = 0x01    # Unit = 1 Degrees Celsius
    frame[ VSCP_MULTICAST_PACKET0_POS_VSCP_DATA + 4 ] = 0x31    # "1"
    frame[ VSCP_MULTICAST_PACKET0_POS_VSCP_DATA + 5 ] = 0x32    # "2"
    frame[ VSCP_MULTICAST_PACKET0_POS_VSCP_DATA + 6 ] = 0x33    # "3"
    frame[ VSCP_MULTICAST_PACKET0_POS_VSCP_DATA + 7 ] = 0x2E    # "."
    frame[ VSCP_MULTICAST_PACKET0_POS_VSCP_DATA + 8 ] = 0x34    # "4"
    frame[ VSCP_MULTICAST_PACKET0_POS_VSCP_DATA + 9 ] = 0x35    # "5"
    frame[ VSCP_MULTICAST_PACKET0_POS_VSCP_DATA + 10 ] = 0x36   # "6"
    frame[ VSCP_MULTICAST_PACKET0_POS_VSCP_DATA + 11 ] = 0x37   # "7"
    frame[ VSCP_MULTICAST_PACKET0_POS_VSCP_DATA + 12 ] = 0x38   # "8"

    # Calculate CRC
    frmstr = ''.join('{:02x}'.format(x) for x in frame[ 1:VSCP_MULTICAST_PACKET0_POS_VSCP_DATA + 12 ] )
    framecrc = CRCCCITT().calculate( frmstr )
    print ''.join('{:02x }'.format(x) for x in frame)

    # CRC
    frame[ 1 + VSCP_MULTICAST_PACKET0_HEADER_LENGTH + 13 ] = (framecrc >> 8) & 0xff
    frame[ 1 + VSCP_MULTICAST_PACKET0_HEADER_LENGTH + 13 + 1 ] = framecrc & 0xff

    #shrink to frame size
    frame = frame[0: (1 + VSCP_MULTICAST_PACKET0_HEADER_LENGTH + 13 + 2 ) ]

    return frame

################################################################################
# sender
#

def sender( group ):
    print("Sender\n")

    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
    sock.setsockopt( socket.IPPROTO_IP, socket.IP_MULTICAST_TTL, 1 )

    while True:
        print(".")
        data = repr( time.time() )
        frame = makeVscpFrame( 0 )
        sock.sendto( frame, (MCAST_GROUP, MCAST_PORT ) )
        time.sleep( 1 )

def receiver( group ):
    print("Receive\n");

    # Look up multicast group address in name server and find out IP version
    addrinfo = socket.getaddrinfo( group, None )[0]

    # Create a socket
    sock = socket.socket(addrinfo[0], socket.SOCK_DGRAM)

    # Allow multiple copies of this program on one machine
    # (not strictly needed)
    sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)

    # Bind it to the port
    sock.bind(('', MYPORT))

    group_bin = socket.inet_pton(addrinfo[0], addrinfo[4][0])
    # Join group
    if addrinfo[0] == socket.AF_INET: # IPv4
        mreq = group_bin + struct.pack('=I', socket.INADDR_ANY)
        sock.setsockopt(socket.IPPROTO_IP, socket.IP_ADD_MEMBERSHIP, mreq)
    else:
        mreq = group_bin + struct.pack('@I', 0)
        sock.setsockopt(socket.IPPROTO_IPV6, socket.IPV6_JOIN_GROUP, mreq)

    # Loop, printing any data we receive
    while True:
        data, sender = sock.recvfrom(1500)
        while data[-1:] == '\0': data = data[:-1] # Strip trailing \0's
        print (str(sender) + '  ' + repr(data))

if __name__ == '__main__':
    main()
