#!/usr/bin/env python

"""
VSCP daemon interface

This python module demonstrates how to communicate with the vscpd daemon trough the telnet interface.

// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
// canald.cpp
//
// This file is part of the VSCP Project (http://www.vscp.org)
//
// Copyright (C) 2009 Thomas Schulz <tschulz@iprimus.com.au>
//
// This file is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this file see the file COPYING.  If not, write to
// the Free Software Foundation, 59 Temple Place - Suite 330,
// Boston, MA 02111-1307, USA.

"""


import sys
import telnetlib
import socket


HOST = "localhost"
PORT = 9598
USER = "admin"
PASSWORD = "secret"
TIMEOUT = 5

TN = None

def open():
    global TN
    try:
        TN = telnetlib.Telnet(HOST, PORT)
        TN.read_until("+OK - Success.", TIMEOUT)
        TN.write("user %s\n" % (USER))
        TN.read_until("password please", TIMEOUT)
        TN.write("pass %s\n" % (PASSWORD))
        TN.read_until("+OK - Success.", TIMEOUT)
    except socket.error:
        print "Could not create Telnet object"
        return False
    return True


def close():
    if not TN:
        return
    TN.write("quit\n")
    TN.close()


def write(cmd):
    if not TN:
        return
    TN.write(cmd)


def get_response():
    """Returns: (status, response)"""

    idx, match, cmd_response = TN.expect(["\-OK", "\+OK"], TIMEOUT)
    TN.read_eager() # Flush out the rest of the line
    return idx, cmd_response


def get_status():
    TN.write("stat\n")
    status, response = get_response()
    #print "stat rv: %d str: %s" % ( status, response.strip())
    return response


def send_burst(number):
    for i in range(number):
        send_string = "SEND 0,20,3,0,0,-,0,1,35\n"
        TN.write(send_string)
        TN.expect(["\+OK", "\-OK"], TIMEOUT)


def get_telnet_object():
    return TN


def main(sys_argv):
    print "vscpd interface main"

    if not open():
        print "Could not open connection to vscpd"
        return
    get_status()
    send_burst(10)
    close()


if __name__ == "__main__":
    main(sys.argv);
