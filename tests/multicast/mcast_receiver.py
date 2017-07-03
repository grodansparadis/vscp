#!/usr/bin/env python
#
# Send/receive UDP multicast packets.
# Requires that your OS kernel supports IP multicast.
#
# Usage:
#   mcast_receiver -s (sender, IPv4)
#   mcast -g <group> -p <port>
#   mcast --group=<group> --port=<port>
#   mcast --help
#  

import time
import struct
import socket
import sys
import getopt
import datetime
import array
import binascii
from ctypes import *
from vscp import *
# pip install pycrc
from PyCRC.CRCCCITT import CRCCCITT
# http://pythonhosted.org/pycrypto/
from Crypto.Hash import SHA256
from Crypto.Cipher import AES
from Crypto import Random

MCAST_PORT = 44444
MCAST_GROUP = '224.0.23.158'


################################################################################
# main
#

def main( argv ):

    group = VSCP_MULTICAST_IPV4_ADDRESS_STR
    port = VSCP_DEFAULT_MULTICAST_PORT
    encryption = VSCP_ENCRYPTION_AES128

    print("VSCP multicast receive test app.\n")

    try:
        opts, args = getopt.getopt(argv,"hg:",["group=","port="])
    except getopt.GetoptError:
        print 'mcast_receive.py -g <group> -p <port>'
        sys.exit(2)
    for opt, arg in opts:
        if opt == '-h':
            print 'mcast_receive.py -g <group> -p <port>'
            sys.exit()
        elif opt in ("-g", "--group"):
            group = arg

    print "Listening on"
    print "------------"  
    print "Group = ", group
    print "Port = ", port
    print "\n"

    receiver( group, port )

################################################################################
# decryptFrame
#

def decryptFrame( encryption, key, frame ):
    sizeData = ( frame[ VSCP_MULTICAST_PACKET0_POS_VSCP_SIZE_MSB ] << 8 ) + \
                        frame[ VSCP_MULTICAST_PACKET0_POS_VSCP_SIZE_LSB ]
    cipher = AES.new( key, AES.MODE_CBC, iv )
    iv = frame[-16:]
    frame = cipher.decrypt( str( frame[1:1 + \
                                    VSCP_MULTICAST_PACKET0_HEADER_LENGTH +\
                                    sizeData + 2 + 1 ] ) )
    return frame

################################################################################
# getVscpEvent
#

def getVscpEvent( frame ):
        
    ev = vscpEventEx()

    if ( VSCP_ENCRYPTION_NONE == ( frame[VSCP_MULTICAST_PACKET0_POS_PKTTYPE] & 0x07 ) ):
        print "Unencrypted frame"
    elif ( VSCP_ENCRYPTION_AES128 == ( frame[VSCP_MULTICAST_PACKET0_POS_PKTTYPE] & 0x07 ) ):
        print "AES128 encrypted frame"
        key = binascii.unhexlify( VSCP_DEFAULT_KEY16 )
        frame = decryptFrame( ( frame[0] & 0x07 ), key, frame)
    elif ( VSCP_ENCRYPTION_AES192 == ( frame[VSCP_MULTICAST_PACKET0_POS_PKTTYPE] & 0x07 ) ):
        print "AES192 encrypted frame"
        key = binascii.unhexlify( VSCP_DEFAULT_KEY24 )
        frame = decryptFrame( ( frame[0] & 0x07 ), key, frame)
    elif ( VSCP_ENCRYPTION_AES256 == ( frame[VSCP_MULTICAST_PACKET0_POS_PKTTYPE] & 0x07 ) ):
        print "AES256 encrypted frame"
        key = binascii.unhexlify( VSCP_DEFAULT_KEY32 )
        frame = decryptFrame( ( frame[0] & 0x07 ), key, frame)
    else :
        print "unknown encrypted frame"
    
    # head
    ev.head = ( frame[ VSCP_MULTICAST_PACKET0_POS_HEAD_MSB ] << 8 ) + \
                        frame[ VSCP_MULTICAST_PACKET0_POS_HEAD_LSB ]

    # timestamp
    ev.timestamp = ( frame[ VSCP_MULTICAST_PACKET0_POS_TIMESTAMP ] << 24 ) + \
                        ( frame[ VSCP_MULTICAST_PACKET0_POS_TIMESTAMP + 1 ] << 16 ) + \
                        ( frame[ VSCP_MULTICAST_PACKET0_POS_TIMESTAMP + 2 ] << 8 ) + \
                        frame[ VSCP_MULTICAST_PACKET0_POS_TIMESTAMP + 3 ]

    # year
    ev.year = ( frame[ VSCP_MULTICAST_PACKET0_POS_YEAR_MSB ] << 8 ) + \
                        frame[ VSCP_MULTICAST_PACKET0_POS_YEAR_LSB ]

    # month
    ev.month = frame[ VSCP_MULTICAST_PACKET0_POS_MONTH ]

    # day
    ev.day = frame[ VSCP_MULTICAST_PACKET0_POS_DAY ]

    # hour
    ev.hour = frame[ VSCP_MULTICAST_PACKET0_POS_HOUR ]

    # minute
    ev.minute = frame[ VSCP_MULTICAST_PACKET0_POS_MINUTE ]

    # second
    ev.second = frame[ VSCP_MULTICAST_PACKET0_POS_SECOND ]

    # class
    ev.vscpclass = ( frame[ VSCP_MULTICAST_PACKET0_POS_VSCP_CLASS_MSB ] << 8 ) + \
                        frame[ VSCP_MULTICAST_PACKET0_POS_VSCP_CLASS_LSB ]

    # type
    ev.vscptype = ( frame[ VSCP_MULTICAST_PACKET0_POS_VSCP_TYPE_MSB ] << 8 ) + \
                        frame[ VSCP_MULTICAST_PACKET0_POS_VSCP_TYPE_LSB ]

    # guid    
    for idx in range (0,15) :
        ev.guid[idx] = frame[ VSCP_MULTICAST_PACKET0_POS_VSCP_GUID + idx ] 

    # size
    ev.sizeData = ( frame[ VSCP_MULTICAST_PACKET0_POS_VSCP_SIZE_MSB ] << 8 ) + \
                        frame[ VSCP_MULTICAST_PACKET0_POS_VSCP_SIZE_LSB ]

    # data
    for idx in range (0,ev.sizeData) : 
                        ev.data[idx] = frame[ VSCP_MULTICAST_PACKET0_POS_VSCP_DATA + idx ] 

    return ev                        


################################################################################
# receiver
#

def receiver( group, port ):

    # Look up multicast group address in name server and find out IP version
    addrinfo = socket.getaddrinfo( group, None )[0]

    # Create a socket
    sock = socket.socket(addrinfo[0], socket.SOCK_DGRAM)

    # Allow multiple copies of this program on one machine
    # (not strictly needed)
    sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)

    # Bind it to the port
    sock.bind(('', port))

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
        ev = vscpEventEx()
        frame, sender = sock.recvfrom(1500)
        if len(frame) < (1 + VSCP_MULTICAST_PACKET0_HEADER_LENGTH + 2) :
            print "Frame have wrong length for a VSCP frame"
            continue
        frame = bytearray( frame )
        ev = getVscpEvent( frame )
        print ( str( sender ) + '  ' + binascii.hexlify( frame ) )
        print "Class=%s Type=%d Size=%d (%d)" % (ev.vscpclass, ev.vscptype, ev.sizeData, len(frame))
        print "\n"

if __name__ == '__main__':
    main(sys.argv[1:])
