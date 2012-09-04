#!/usr/bin/env python

"""
Read registers from a node connected to the vscp daemon.
This module can be used to test the speed of the register update.

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
import time

import vscpdif
from vscp_class import *


def page_read(node, page, num):
    
    seq_nos = {}

    vscpdif.send(class1=VSCP_CLASS1_PROTOCOL, type=24, data=[node, page, num]) # Page Read
    while True:
        response = vscpdif.get_response_match(class1=VSCP_CLASS1_PROTOCOL, type=26)
        if response:
            seq_no = response.data[0]
            seq_nos[seq_no] = response.data
            #print "seq_no: ", seq_no, " data:",
            #print ",".join(["%02x"%(el) for el in response.data[1:]]) # print as data arrives
        else:
            break
    
    # Check that all sequence numbers are there
    n = int(num / 7) + 1  # number of expected response events

    if False:
        for i in range(n):
            if i not in seq_nos:
                print "missing seq_no: ", i

    if True:
        for i in range(n):
            try:
                v = seq_nos[i]
                print "seq_no: %d   " % (i), ",".join(["%02x"%(el) for el in v[1:]])
            except KeyError:
                print "seq_no: %d  missing" % (i)
            

def write_register(node, reg, val):

    vscpdif.send(class1=VSCP_CLASS1_PROTOCOL, type=11, data=[node, reg, val]) # Write Register
    response = vscpdif.get_response_match(class1=VSCP_CLASS1_PROTOCOL, type=10)
    if not response:
        print "no response"
        return False

    # Decode response and extract register and value
    register = int(response.data[0])
    value = int(response.data[1])
    if reg != register:
        print "ERROR register"
        return False
    if val != value:
        print "ERROR value"
        return False

    return True


def read_register(node, reg):

    vscpdif.send(class1=VSCP_CLASS1_PROTOCOL, type=9, data=[node, reg]) # Read Register
    response = vscpdif.get_response_match(class1=VSCP_CLASS1_PROTOCOL, type=10)
    if not response:
        #print "no response"
        return None

    # Decode response and extract register and value
    register = int(response.data[0])
    value = int(response.data[1])

    # Cross check
    if reg <> register:
        print "ERROR read_register"
        return None

    return value


def performance_test(node):
    """Read 255 registers from (node) and print time."""

    n_reg = 255
    t_start = time.time()
    for i in range(n_reg):
        read_register(node, i)
    t_stop = time.time()
    print "Read of %d registers takes %.1f seconds" % (n_reg, t_stop-t_start)


def usage():
    s = "Usage:\n"
    s += "  read_register <node> <register>\n"
    s += "     Read <register> from <node>\n"
    s += "  read_register perf <node>\n"
    s += "     Read all 255 registers from node <node> and print time used."
    return s


def main(sys_argv):

    for arg in sys_argv:
        if arg in ["-h", "--help"]:
            print usage()
            return

    if not vscpdif.open():
        print "vscpdif.open failed"
        return

    if sys.argv[1] == "perf":
        node = int(sys.argv[2])
        performance_test(node)
        return

    if len(sys.argv) == 3:
        # Read one register
        node = int(sys.argv[1])
        reg = int(sys.argv[2])
        val = read_register(node, reg)
        if val != None:
            print "read_register node: %d register: %d ==> val: %s" % (node, reg, val)
        else:
            print "read_register failed"
        return


if __name__ == "__main__":
    main(sys.argv);
