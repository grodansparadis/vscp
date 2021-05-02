# FILE: vscp.py
#
# This file is part of the VSCP (https://www.vscp.org) 
#
# The MIT License (MIT)
# 
# Copyright (c) 2000-2017 Ake Hedman, the VSCP project <akhe@vscp.org>
# 
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
# 
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

import time
import struct
import socket
import sys
import datetime
from ctypes import *

VSCP_DEFAULT_UDP_PORT =                 33333
VSCP_DEFAULT_TCP_PORT =                 9598
VSCP_ANNNOUNCE_MULTICAST_PORT =         9598
VSCP_MULTICAST_IPV4_ADDRESS_STR =       "224.0.23.158"
VSCP_DEFAULT_MULTICAST_PORT =           44444
VSCP_DEFAULT_MULTICAST_TTL =            1

VSCP_ADDRESS_SEGMENT_CONTROLLER	=       0x00
VSCP_ADDRESS_NEW_NODE =                 0xff

#VSCP levels
VSCP_LEVEL1 =                           0
VSCP_LEVEL2 =                           1

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
                ("vscpclass", c_ushort),
                ("vscptype", c_ushort),
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
                ("vscpclass", c_ushort),
                ("vscptype", c_ushort),
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

# Packet frame format type = 0
#      without byte0 and CRC
#      total frame size is 1 + 34 + 2 + data-length
VSCP_MULTICAST_PACKET0_HEADER_LENGTH =  35

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
# Two byte CRC follow here and if the frame is encrypted
# the initialization vector follows.

# VSCP multicast packet types
VSCP_MULTICAST_TYPE_EVENT = 0

# Multicast proxy CLASS=1026, TYPE=3 https://www.vscp.org/docs/vscpspec/doku.php?id=class2.information#type_3_0x0003_level_ii_proxy_node_heartbeat
VSCP_MULTICAST_PROXY_HEARTBEAT_DATA_SIZE      =     192
VSCP_MULTICAST_PROXY_HEARTBEAT_POS_REALGUID   =     0       # The real GUID for the node
VSCP_MULTICAST_PROXY_HEARTBEAT_POS_IFGUID     =     32      # GUID for interface node is on
VSCP_MULTICAST_PROXY_HEARTBEAT_POS_IFLEVEL    =     48      # 0=Level I node, 1=Level II node
VSCP_MULTICAST_PROXY_HEARTBEAT_POS_NODENAME   =     64      # Name of node
VSCP_MULTICAST_PROXY_HEARTBEAT_POS_IFNAME     =     128     # Name of interface

# Default key for VSCP Server
# Change if other key is used
VSCP_DEFAULT_KEY16 = 'A4A86F7D7E119BA3F0CD06881E371B98'
VSCP_DEFAULT_KEY24 = 'A4A86F7D7E119BA3F0CD06881E371B989B33B6D606A863B6'
VSCP_DEFAULT_KEY32 = 'A4A86F7D7E119BA3F0CD06881E371B989B33B6D606A863B633EF529D64544F8E'

# Bootloaders
VSCP_BOOTLOADER_VSCP          =         0x00	# VSCP boot loader algorithm
VSCP_BOOTLOADER_PIC1          =         0x01	# PIC algorithm 0
VSCP_BOOTLOADER_AVR1          =         0x10	# AVR algorithm 0
VSCP_BOOTLOADER_LPC1          =         0x20	# NXP/Philips LPC algorithm 0
VSCP_BOOTLOADER_ST            =         0x30	# ST STR algorithm 0
VSCP_BOOTLOADER_FREESCALE     =         0x40	# Freescale Kinetics algorithm 0
VSCP_BOOTLOADER_NONE          =         0xff

#          * * * Data Coding for VSCP packets * * *

# Data format masks
VSCP_MASK_DATACODING_TYPE     =         0xE0  # Bits 5,6,7
VSCP_MASK_DATACODING_UNIT     =         0x18  # Bits 3,4
VSCP_MASK_DATACODING_INDEX    =         0x07  # Bits 0,1,2

# These bits are coded in the three MSB bytes of the first data byte
# in a packet and tells the type of the data that follows.
VSCP_DATACODING_BIT           =         0x00
VSCP_DATACODING_BYTE          =         0x20
VSCP_DATACODING_STRING        =         0x40
VSCP_DATACODING_INTEGER       =         0x60
VSCP_DATACODING_NORMALIZED    =         0x80
VSCP_DATACODING_SINGLE        =         0xA0  # single precision float
VSCP_DATACODING_RESERVED1     =         0xC0
VSCP_DATACODING_RESERVED2     =         0xE0

# These bits are coded in the four least significant bits of the first data byte
# in a packet and tells how the following data should be interpreted. For a flow sensor
# the default format can be litres/minute. Other formats such as m3/second can be defined 
# by the node if it which. However it must always be able to report in the default format.
VSCP_DATACODING_INTERPRETION_DEFAULT  =  0

# CRC8 Constants
VSCP_CRC8_POLYNOMIAL          =         0x18
VSCP_CRC8_REMINDER            =         0x00

# CRC16 Constants
VSCP_CRC16_POLYNOMIAL         =         0x1021
VSCP_CRC16_REMINDER           =         0xFFFF

# CRC32 Constants
VSCP_CRC32_POLYNOMIAL         =         0x04C11DB7
VSCP_CRC32_REMINDER           =         0xFFFFFFFF


# Node data - the required registers are fetched from this 
#	structure
class vscpMyNode(Structure):
    _fields_ = [ ("guid", c_ubyte * 16),
                 ("nicknameID", c_ubyte ) ]

# * * * Standard VSCP registers * * *

# Register defines above 0x7f
VSCP_STD_REGISTER_ALARM_STATUS              =   0x80

VSCP_STD_REGISTER_MAJOR_VERSION             =   0x81
VSCP_STD_REGISTER_MINOR_VERSION             =   0x82
VSCP_STD_REGISTER_SUB_VERSION               =   0x83

# 0x84 - 0x88
VSCP_STD_REGISTER_USER_ID                   =   0x84

# 0x89 - 0x8C
VSCP_STD_REGISTER_USER_MANDEV_ID            =   0x89

# 0x8D -0x90
VSCP_STD_REGISTER_USER_MANSUBDEV_ID         =   0x8D

# Nickname
VSCP_STD_REGISTER_NICKNAME_ID               =   0x91

# Selected register page
VSCP_STD_REGISTER_PAGE_SELECT_MSB           =   0x92
VSCP_STD_REGISTER_PAGE_SELECT_LSB           =   0x93

# Firmware version
VSCP_STD_REGISTER_FIRMWARE_MAJOR            =   0x94
VSCP_STD_REGISTER_FIRMWARE_MINOR            =   0x95
VSCP_STD_REGISTER_FIRMWARE_SUBMINOR         =   0x96

VSCP_STD_REGISTER_BOOT_LOADER               =   0x97
VSCP_STD_REGISTER_BUFFER_SIZE               =   0x98
VSCP_STD_REGISTER_PAGES_COUNT               =   0x99

# 0xd0 - 0xdf  GUID
VSCP_STD_REGISTER_GUID                      =   0xD0
 
# 0xe0 - 0xff  MDF
VSCP_STD_REGISTER_DEVICE_URL                =   0xE0

# Level I Decision Matrix
VSCP_LEVEL1_DM_ROW_SIZE                     =   8

VSCP_LEVEL1_DM_OFFSET_OADDR                 =   0
VSCP_LEVEL1_DM_OFFSET_FLAGS                 =   1
VSCP_LEVEL1_DM_OFFSET_CLASS_MASK            =   2
VSCP_LEVEL1_DM_OFFSET_CLASS_FILTER          =   3
VSCP_LEVEL1_DM_OFFSET_TYPE_MASK             =   4
VSCP_LEVEL1_DM_OFFSET_TYPE_FILTER           =   5
VSCP_LEVEL1_DM_OFFSET_ACTION                =   6
VSCP_LEVEL1_DM_OFFSET_ACTION_PARAM          =   7

# Bits for VSCP server 64/16-bit capability code
# used by CLASS1.PROTOCOL, HIGH END SERVER RESPONSE
# and low end 16-bits for
# CLASS2.PROTOCOL, HIGH END SERVER HEART BEAT

VSCP_SERVER_CAPABILITY_TCPIP                =   (1<<15)
VSCP_SERVER_CAPABILITY_UDP                  =   (1<<14)
VSCP_SERVER_CAPABILITY_MULTICAST_ANNOUNCE   =   (1<<13)
VSCP_SERVER_CAPABILITY_RAWETH               =   (1<<12)
VSCP_SERVER_CAPABILITY_WEB                  =   (1<<11)
VSCP_SERVER_CAPABILITY_WEBSOCKET            =   (1<<10)
VSCP_SERVER_CAPABILITY_REST                 =   (1<<9)
VSCP_SERVER_CAPABILITY_MULTICAST_CHANNEL    =   (1<<8)
VSCP_SERVER_CAPABILITY_RESERVED             =   (1<<7)
VSCP_SERVER_CAPABILITY_IP6                  =   (1<<6)
VSCP_SERVER_CAPABILITY_IP4                  =   (1<<5)
VSCP_SERVER_CAPABILITY_SSL                  =   (1<<4)
VSCP_SERVER_CAPABILITY_TWO_CONNECTIONS      =   (1<<3)
VSCP_SERVER_CAPABILITY_AES256               =   (1<<2)
VSCP_SERVER_CAPABILITY_AES192               =   (1<<1)
VSCP_SERVER_CAPABILITY_AES128               =   1

# Offsets into the data of the capabilities event
# VSCP_CLASS2_PROTOCOL, Type=20/VSCP2_TYPE_PROTOCOL_HIGH_END_SERVER_CAPS
VSCP_CAPABILITY_OFFSET_CAP_ARRAY            =   0
VSCP_CAPABILITY_OFFSET_GUID                 =   8
VSCP_CAPABILITY_OFFSET_IP_ADDR              =   24
VSCP_CAPABILITY_OFFSET_SRV_NAME             =   40
VSCP_CAPABILITY_OFFSET_NON_STD_PORTS        =   104

# Error Codes
VSCP_ERROR_SUCCESS                          =   0       # All is OK
VSCP_ERROR_ERROR                            =   -1      # Error
VSCP_ERROR_CHANNEL                          =   7       # Invalid channel
VSCP_ERROR_FIFO_EMPTY                       =   8       # FIFO is empty
VSCP_ERROR_FIFO_FULL                        =   9       # FIFI is full
VSCP_ERROR_FIFO_SIZE                        =   10      # FIFO size error
VSCP_ERROR_FIFO_WAIT                        =   11      
VSCP_ERROR_GENERIC                          =   12      # Generic error
VSCP_ERROR_HARDWARE                         =   13      # Hardware error
VSCP_ERROR_INIT_FAIL                        =   14      # Initialization failed
VSCP_ERROR_INIT_MISSING                     =   15
VSCP_ERROR_INIT_READY                       =   16
VSCP_ERROR_NOT_SUPPORTED                    =   17      # Not supported
VSCP_ERROR_OVERRUN                          =   18      # Overrun
VSCP_ERROR_RCV_EMPTY                        =   19      # Receive buffer empty
VSCP_ERROR_REGISTER                         =   20      # Register value error
VSCP_ERROR_TRM_FULL                         =   21      # Transmit buffer full
VSCP_ERROR_LIBRARY                          =   28      # Unable to load library
VSCP_ERROR_PROCADDRESS                      =   29      # Unable get library proc. address
VSCP_ERROR_ONLY_ONE_INSTANCE                =   30      # Only one instance allowed
VSCP_ERROR_SUB_DRIVER                       =   31      # Problem with sub driver call
VSCP_ERROR_TIMEOUT                          =   32      # Time-out
VSCP_ERROR_NOT_OPEN                         =   33      # The device is not open.
VSCP_ERROR_PARAMETER                        =   34      # A parameter is invalid.
VSCP_ERROR_MEMORY                           =   35      # Memory exhausted.
VSCP_ERROR_INTERNAL                         =   36      # Some kind of internal program error
VSCP_ERROR_COMMUNICATION                    =   37      # Some kind of communication error
VSCP_ERROR_USER                             =   38      # Login error user name
VSCP_ERROR_PASSWORD                         =   39      # Login error password
VSCP_ERROR_CONNECTION                       =   40      # Could not connect   
VSCP_ERROR_INVALID_HANDLE                   =   41      # The handle is not valid
VSCP_ERROR_OPERATION_FAILED                 =   42      # Operation failed for some reason
VSCP_ERROR_BUFFER_TO_SMALL                  =   43      # Supplied buffer is to small to fit content
VSCP_ERROR_UNKNOWN_ITEM                     =   44      # Requested item (remote variable) is unknown
VSCP_ERROR_ALREADY_DEFINED                  =   45      # The name is already in use.

#
#    Template for VSCP XML event data
# 
#    data: datetime,head,obid,datetime,timestamp,class,type,guid,sizedata,data,note
#  
#<event>    
#    <head>3</head>
#    <obid>1234</obid>
#    <datetime>2017-01-13T10:16:02</datetime>
#    <timestamp>50817</timestamp>
#    <class>10</class>
#    <type>6</type>
#    <guid>00:00:00:00:00:00:00:00:00:00:00:00:00:01:00:02</guid>
#    <sizedata>7</sizedata>
#    <data>0x48,0x34,0x35,0x2E,0x34,0x36,0x34</data>
#    <note></note>
#</event>
# 
VSCP_XML_EVENT_TEMPLATE = "<event>\n"\
    "<head>%d</head>\n"\
    "<obid>%lu</obid>\n"\
    "<datetime>%s</datetime>\n"\
    "<timestamp>%lu</timestamp>\n"\
    "<class>%d</class>\n"\
    "<type>%d</type>\n"\
    "<guid>%s</guid>\n"\
    "<sizedata>%d</sizedata>\n"\
    "<data>%s</data>\n"\
    "<note>%s</note>\n"\
"</event>"

#
#  
#    Template for VSCP JSON event data
#    data: datetime,head,obid,datetime,timestamp,class,type,guid,data,note 
#  
#    
#    "head": 2,
#    "obid"; 123,
#    "datetime": "2017-01-13T10:16:02",
#    "timestamp":50817,
#    "class": 10,
#    "type": 8,
#    "guid": "00:00:00:00:00:00:00:00:00:00:00:00:00:01:00:02",
#    "data": [1,2,3,4,5,6,7],
#    "note": "This is some text"
#
#
VSCP_JSON_EVENT_TEMPLATE = "{\n"\
    "\"head\": %d,\n"\
    "\"obid\":  %lu,\n"\
    "\"datetime\": \"%s\",\n"\
    "\"timestamp\": %lu,\n"\
    "\"class\": %d,\n"\
    "\"type\": %d,\n"\
    "\"guid\": \"%s\",\n"\
    "\"data\": [%s],\n"\
    "\"note\": \"%s\"\n"\
"}"

#
# 
#    Template for VSCP HTML event data  
#   
#    data: datetime,class,type,data-count,data,guid,head,timestamp,obid,note
# 
#<h2>VSCP Event</h2>
#<p>
#    Time: 2017-01-13T10:16:02 <br>
#</p>
#<p>
#    Class: 10 <br>
#    Type: 6 <br>
#</p>
#<p>
#    Data count: 6<br>
#    Data: 1,2,3,4,5,6,7<br>
#</p>
#<p>
#    From GUID: 00:00:00:00:00:00:00:00:00:00:00:00:00:01:00:02<br>
#</p>
#<p>
#    Head: 6 <br>
#    DateTime: 2013-11-02T12:34:22Z
#    Timestamp: 1234 <br>
#    obid: 1234 <br>
#    note: This is a note <br>
#</p>
# 

VSCP_HTML_EVENT_TEMPLATE = "<h2>VSCP Event</h2> "\
    "<p>"\
    "Class: %d <br>"\
    "Type: %d <br>"\
    "</p>"\
    "<p>"\
    "Data count: %d<br>"\
    "Data: %s<br>"\
    "</p>"\
    "<p>"\
    "From GUID: %s<br>"\
    "</p>"\
    "<p>"\
    "Head: %d <br>"\
    "<p>"\
    "DateTime: %s <br>"\
    "</p>"\
    "Timestamp: %lu <br>"\
    "obid: %lu <br>"\
    "note: %s <br>"\
    "</p>"

# Set packet type part of multicast packet type
def SET_VSCP_MULTICAST_TYPE( type, encryption ) :     
    return ( ( type << 4 ) | encryption )

# Get packet type part of multicast packet type
def GET_VSCP_MULTICAST_PACKET_TYPE( type) :   
    return ( ( type >> 4 ) & 0x0f )

# Get encryption part if multicast packet type
def GET_VSCP_MULTICAST_PACKET_ENCRYPTION( type ) :     
    return ( ( type ) & 0x0f )

# Get data coding type
def VSCP_DATACODING_TYPE( b ) :
    return ( VSCP_MASK_DATACODING_TYPE & b )

# Get data coding unit
def VSCP_DATACODING_UNIT( b ) :
    return ( ( VSCP_MASK_DATACODING_UNIT & b ) >> 3 )

# Get data coding sensor index
def VSCP_DATACODING_INDEX( b ) :
    return ( VSCP_MASK_DATACODING_INDEX & b )

