#!/usr/bin/env python

import socket
import struct

def main():
  MCAST_GRP = '224.0.23.158'
  MCAST_PORT = 44444
  sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
  sock.setsockopt(socket.IPPROTO_IP, socket.IP_MULTICAST_TTL, 32)
  sock.sendto('Hello World!', (MCAST_GRP, MCAST_PORT))

if __name__ == '__main__':
  main()
