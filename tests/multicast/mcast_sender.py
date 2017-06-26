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
from ctypes import *
# pip install pycrc
from PyCRC.CRCCCITT import CRCCCITT

MCAST_PORT = 44444
MCAST_GROUP = '224.0.23.158'
MCAST_TTL = 1   # Increase to reach other networks

VSCP_DEFAULT_UDP_PORT =             33333
VSCP_DEFAULT_TCP_PORT =             9598
VSCP_ANNNOUNCE_MULTICAST_PORT =     9598
VSCP_DEFAULT_MULTICAST_PORT =       44444

VSCP_ADDRESS_SEGMENT_CONTROLLER	=   0x00
VSCP_ADDRESS_NEW_NODE =             0xff

VSCP_LEVEL1 =                       0
VSCP_LEVEL2 =                       1

class vscpEvent(Structure):
    _fields_ = [("crc", c_ushort),
                ("obid", c_ulong),
                ("timestamp", c_ulong),
                ("head", c_ushort),
                ("year", c_ushort),
                ("month", c_ubyte),
                ("day", c_ubyte),
                ("hour", c_ubyte),
                ("minute", c_ubyte),
                ("second", c_ubyte),
                ("class", c_ushort),
                ("type", c_ushort),
                ("guid", c_ubyte * 16),
                ("sizeData", c_ushort),
                ("pdata", c_void_p)]

class vscpEventEx(Structure):
    _fields_ = [("crc", c_ushort),
                ("obid", c_ulong),
                ("timestamp", c_ulong),
                ("head", c_ushort),
                ("year", c_ushort),
                ("month", c_ubyte),
                ("day", c_ubyte),
                ("hour", c_ubyte),
                ("minute", c_ubyte),
                ("second", c_ubyte),
                ("class", c_ushort),
                ("type", c_ushort),
                ("guid", c_ubyte * 16),
                ("sizeData", c_ushort),
                ("data", c_ubyte * VSCP_MAX_DATA)]

VSCP_PRIORITY_0 =                       0x00
VSCP_PRIORITY_1 =                       0x20
VSCP_PRIORITY_2 =                       0x40
VSCP_PRIORITY_3 =                       0x60
VSCP_PRIORITY_4 =                       0x80
VSCP_PRIORITY_5 =                       0xA0
VSCP_PRIORITY_6 =                       0xC0
VSCP_PRIORITY_7 =                       0xE0

VSCP_PRIORITY_HIGH =                    0x00
VSCP_PRIORITY_LOW =                     0xE0
VSCP_PRIORITY_MEDIUM =                  0xC0
VSCP_PRIORITY_NORMAL =                  0x60

VSCP_HEADER_PRIORITY_MASK =             0xE0

VSCP_HEADER_HARD_CODED =                0x10    # If set node nickname is hardcoded
VSCP_HEADER_NO_CRC =                    0x08    # Don't calculate CRC

VSCP_NO_CRC_CALC =                      0x08    # If set no CRC is calculated

VSCP_MASK_PRIORITY =                    0xE0
VSCP_MASK_HARDCODED =                   0x10
VSCP_MASK_NOCRCCALC =                   0x08

VSCP_LEVEL1_MAXDATA =                   8
VSCP_LEVEL2_MAXDATA =                   (512 - 25)

VSCP_NOCRC_CALC_DUMMY_CRC =             0xAA55  # If no CRC cal bit is set the CRC value
                                                # should be set to this value for the CRC
                                                # calculation to be skipped.

VSCP_CAN_ID_HARD_CODED =	            0x02000000 # Hard coded bit in CAN frame id

# GUID byte positions
VSCP_GUID_MSB =                         0
VSCP_GUID_LSB =                         15

# Event filter
class vscpEventFilter(Structure):
    _fields_ = [("filter_priority", c_ubyte),
                ("mask_priority", c_ubyte),
                ("filter_class", c_ubyte),
                ("mask_class", c_ubyte),
                ("filter_type", c_ubyte),
                ("mask_type", c_ubyte),
                ("filter_guid", c_ubyte * 16),
                ("mask_guid", c_ubyte * 16) ]

# Transmission statistics structure
class VSCPStatistics(Structure):
    _fields_ = [("cntReceiveFrames", c_long),
                ("cntTransmitFrames", c_long),
                ("cntReceiveData", c_long),
                ("cntTransmitData", c_long),
                ("cntOverruns", c_long),
                ("x", c_long),
                ("y", c_long),
                ("z", c_long) ]

class VSCPStatus(Structure):
    _fields_ = [("channel_status", c_ulong),
                ("lasterrorcode", c_ulong),
                ("lasterrorsubcode", c_ulong)]

class VSCPChannelInfo(Structure):
    _fields_ = [("channelType", c_ubyte),
                ("channel", c_ushort),
                ("guid", c_ubyte * 16)]

# VSCP Encryption types
VSCP_ENCRYPTION_NONE =                  0
VSCP_ENCRYPTION_AES128 =                1
VSCP_ENCRYPTION_AES192 =                2
VSCP_ENCRYPTION_AES256 =                3

# VSCP Encryption tokens
VSCP_ENCRYPTION_TOKEN_0 =               ""
VSCP_ENCRYPTION_TOKEN_1 =               "AES128"
VSCP_ENCRYPTION_TOKEN_2 =               "AES192"
VSCP_ENCRYPTION_TOKEN_3 =               "AES256"

# * * * Multicast on VSCP reserved IP 224.0.23.158

VSCP_MULTICAST_IPV4_ADDRESS_STR =           "224.0.23.158"

# Packet frame format type = 0
#      without byte0 and CRC
#      total frame size is 1 + 34 + 2 + data-length
VSCP_MULTICAST_PACKET0_HEADER_LENGTH =      35

# Multicast packet ordinals
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

    # CRC
    frame[ 1 + VSCP_MULTICAST_PACKET0_HEADER_LENGTH + 13 ] = (framecrc >> 8) & 0xff
    frame[ 1 + VSCP_MULTICAST_PACKET0_HEADER_LENGTH + 13 + 1 ] = framecrc & 0xff

    #shrink to frame size
    frame = frame[0: (1 + VSCP_MULTICAST_PACKET0_HEADER_LENGTH + 13 + 2 ) ]
    print 'Frame = ' + ''.join('{:02x} '.format(x) for x in frame)

    return frame

################################################################################
# sender
#

def sender( group ):
    print("Sender\n")

    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
    sock.setsockopt( socket.IPPROTO_IP, socket.IP_MULTICAST_TTL, 1 )

    while True:
        data = repr( time.time() )
        frame = makeVscpFrame( 0 )
        sock.sendto( frame, (MCAST_GROUP, MCAST_PORT ) )
        time.sleep( 1 )


if __name__ == '__main__':
    main()
