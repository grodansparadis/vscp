import time
import struct
import socket
import sys
import datetime
from ctypes import *

VSCP_DEFAULT_UDP_PORT =             33333
VSCP_DEFAULT_TCP_PORT =             9598
VSCP_ANNNOUNCE_MULTICAST_PORT =     9598
VSCP_DEFAULT_MULTICAST_PORT =       44444

VSCP_ADDRESS_SEGMENT_CONTROLLER	=   0x00
VSCP_ADDRESS_NEW_NODE =             0xff

#VSCP levels
VSCP_LEVEL1 =                       0
VSCP_LEVEL2 =                       1

# VSCP priority
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

# VSCP event structure
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

# VSCP event ex structure
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
                ("data", c_ubyte * VSCP_LEVEL2_MAXDATA)]

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
