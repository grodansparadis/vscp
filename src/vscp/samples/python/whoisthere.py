#!/usr/bin/env python

"""
Check which nodes are connected to the daemon.

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
import time
from optparse import OptionParser

import vscpdif
from vscp_class import *


def whoisthere():

    guids = {}
    vscpdif.send(class1=VSCP_CLASS1_PROTOCOL, type=31, data=[0xff]) # Who is there?
    while True:
        response = vscpdif.get_response_match(class1=VSCP_CLASS1_PROTOCOL, type=32, tries=10)
        if not response:
            break
        if response.guid not in guids:
            print "detected: ", response.guid
        guids[response.guid] = True

    return guids


def probe(nn):
    """Probe for one node with nickname nn"""

    vscpdif.send(class1=VSCP_CLASS1_PROTOCOL, type=2, data=[nn]) # Probe
    response = vscpdif.get_response_match(class1=VSCP_CLASS1_PROTOCOL, type=3, tries=100, delay=0.001) # Probe ACK
    if response:
        return True

    return False


def probe_all():

    for nickname in range(1,10):
        status = probe(nickname)
        sys.stdout.write("\rnode %d " % (nickname))
        if status:
            print "is online"
        #time.sleep(0.1)
    sys.stdout.write("\r                      ")


def main():

    parser = OptionParser()
    parser.add_option("-p", "--probe",
                        action="store_true", dest="probe",
                        help="Send probe events")
    options, args = parser.parse_args()


    if not vscpdif.open():
        print "vscpdif.open failed"
        return

    if options.probe:
        probe_all()
    else:
        whoisthere()


if __name__ == "__main__":
    main();
