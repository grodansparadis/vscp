#!/usr/bin/env python

"""
This python program demonstrates how to communicate with the vscpd daemon trough the telnet interface.

This program has been created to test the data troughput of the daemon by sending a number of messages. 
It is not intended to be bullet proof or to be fit for any purpose.


// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
// canald.cpp
//
// This file is part of the VSCP Project (https://www.vscp.org)
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


HOST = "localhost"
PORT = 9598
USER = "admin"
PASSWORD = "secret"
TIMEOUT = 5

TN = None

def vscpd_open():
    global TN
    TN = telnetlib.Telnet(HOST, PORT)
    TN.read_until("+OK - Success.", TIMEOUT)
    TN.write("user %s\n" % (USER))
    TN.read_until("password please", TIMEOUT)
    TN.write("pass %s\n" % (PASSWORD))
    TN.read_until("+OK - Success.", TIMEOUT)


def vscpd_close():
    TN.write("quit\n")
    TN.close()


def vscpd_get_status():
    TN.write("stat\n")
    idx, match, cmd_response = TN.expect(["\+OK", "\-OK"], TIMEOUT)
    print "stat response: ", cmd_response.strip()


def vscpd_send_burst(number):
    for i in range(number):
        send_string = "SEND 0,20,3,0,0,-,0,1,35\n"
        TN.write(send_string)
        TN.expect(["\+OK", "-OK"], TIMEOUT)


def main(sys_argv):
    print "send_burst"

    num_send = 10
    if len(sys_argv) > 1:
        num_send = int(sys_argv[1])

    vscpd_open()
    vscpd_send_burst(num_send)
    vscpd_close()


if __name__ == "__main__":
    main(sys.argv);
