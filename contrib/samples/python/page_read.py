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
// Copyright (C) 2011 Thomas Schulz <tschulz@iprimus.com.au>
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
import vscpdif
import read_register


def usage():
    s = "Usage:\n"
    s += "  page_read <node> <page> <num>\n"
    return s


def main(sys_argv):

    for arg in sys_argv:
        if arg in ["-h", "--help"]:
            print usage()
            return

    if not vscpdif.open():
        print "vscpdif.open failed"
        return


    if len(sys.argv) == 4:
        node = int(sys.argv[1])
        page = int(sys.argv[2])
        num = int(sys.argv[3])
        val = read_register.page_read(node, page, num)
        return


if __name__ == "__main__":
    main(sys.argv);
