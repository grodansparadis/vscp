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


NODE = 2

def read_register(reg=5):

    # Query register
    vscpdif.write("SEND 0,0,9,0,0,-,%d,%d\n" % (NODE, reg)) # Read from node (node) register (reg)
    vscpdif.get_response()

    # Wait for response
    for i in range(100):
        vscpdif.write("retr\n")
        rv, response = vscpdif.get_response()
        if rv:
            print "response (%d) <<<%s>>>" % (i, response)
            break
        time.sleep(0.01)


def main(sys_argv):
    print "read_register"

    n_reg = 100
    if not vscpdif.open():
        print "vscpdif.open failed"
        return

    t_start = time.time()
    for i in range(n_reg):
        read_register(i)
    t_stop = time.time()
    print "Read of %d registers takes %.1f seconds" % (n_reg, t_stop-t_start)
    vscpdif.close()


if __name__ == "__main__":
    main(sys.argv);
