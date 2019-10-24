#!/usr/bin/env python

"""
VSCP daemon interface

This python module demonstrates how to communicate with the vscpd daemon trough the TCP/IP interface.

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
import time


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


def get_response(timeout=TIMEOUT):
    """Returns: (status, response)"""

    idx, match, cmd_response = TN.expect(["\-OK", "\+OK"], timeout)
    TN.read_eager() # Flush out the rest of the line
    r = cmd_response.split()
    #print "get_response idx: %d resp: <<<%s>>>" % (idx, cmd_response)
    
    if idx == -1:
        return 0, ""
    
    if len(r) > 1:
        response_str = r[0]
    else:
        response_str=cmd_response
    return idx, response_str


def get_status():
    return vscpd_command("stat\n")


def vscpd_command(command):
    TN.write("%s\n" % command)
    status, response = get_response()
    if status:
        return response
    return None


def get_telnet_object():
    return TN


def send(class1=511, type=1, data=[]):
    data_str = ",".join([str(el) for el in data])
    write("SEND 0,%d,%d,0,0,-,%s\n" % (class1, type, data_str))


class vscp_rx_event(object):
    """Take the string from the vscp daemon and decode it."""

    class1 = None
    type = None
    guid = None
    n_data = 0
    data = []
    
    def __init__(self, retr_string):
        f = retr_string.split(",")
        try:
            self.class1 = int(f[1])
            self.type = int(f[2])
            self.guid = f[5]
            n_data = len(f)-6
            self.data = []
            for i in range(n_data):
                self.data.append(int(f[i+6]))
        except:
            pass
            
            
    def __str__(self):
        data_str = ",".join([str(el) for el in self.data])
        return "vscp_rx_event c: %s t: %s  [%s]" % (self.class1, self.type, data_str)

        
def get_response_match(class1, type, oaddr=None, tries=100, delay=0.01):

    rv, response = get_response(timeout=delay)
    #print "rv: %d, resp: %s" % (rv, response)

    # Wait for response
    for i in range(tries):
        write("retr\n")
        rv, response = get_response()
        if rv:
            #print "response (%d) <<<%s>>>" % (i, response)
            obj = vscp_rx_event(response)
            #print obj
            if obj.class1 != class1:
                #print "no match (class)"
                continue
            if obj.type != type:
                #print "no match (type)"
                continue
            # oaddr test not implemented yet
            
            # Hit
            break
        time.sleep(delay)
    else:
        #print "no response"
        return None
        
    return obj


def main(sys_argv):
    print "vscpd interface main"

    if not open():
        print "Could not open connection to vscpd"
        return
    print "Connection to vscpd has been established"

    print "vscpd stat: %s" % (vscpd_command("stat"))
    print "vscpd vers: %s" % (vscpd_command("vers"))
    print "vscpd info: %s" % (vscpd_command("info"))
    print "vscpd channel id: %s" % (vscpd_command("chid"))
    print "vscpd channel guid: %s" % (vscpd_command("ggid"))

    print "Closing connection to vscpd"
    close()


if __name__ == "__main__":
    main(sys.argv);
