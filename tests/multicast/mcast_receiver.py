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

    print "Group = ", group
    print "Port = ", port

    receiver( group, port )

################################################################################
# getVscpEvent
#

def getVscpEvent( frame, ev ):
        
    if ( VSCP_ENCRYPTION_NONE == ( frame[0] & 0x07 ) ):
        print "Unencrypted frame"
    elif ( VSCP_ENCRYPTION_AES128 == ( frame[0] & 0x07 ) ):
        print "AES128 encrypted frame"
    elif ( VSCP_ENCRYPTION_AES192 == ( frame[0] & 0x07 ) ):
        print "AES192 encrypted frame"
    elif ( VSCP_ENCRYPTION_AES256 == ( frame[0] & 0x07 ) ):
        print "AES256 encrypted frame"
    else :
        print "unknown encrypted frame"
    
    # head
    ev.head = frame[ VSCP_MULTICAST_PACKET0_POS_HEAD_MSB ] << 8 + \
        frame[ VSCP_MULTICAST_PACKET0_POS_HEAD_LSB ]


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
        frame, sender = sock.recvfrom(1500)
        SizeDate = len(frame)
        #while data[-1:] == '\0': data = data[:-1] # Strip trailing \0's
        print ( str( sender ) + '  ' + binascii.hexlify( frame ) )
        ev = vscpEvent()
        frame = bytearray( frame )
        getVscpEvent( frame, ev )

if __name__ == '__main__':
    main(sys.argv[1:])
