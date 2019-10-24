#!/usr/bin/env python

"""
A Python VSCP Client.

* Connects to vscpd via TCP/IP interface.
* Implements most important level-1 protocol events.
* Stores registers in a file.
* Requires python 2.7



// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
// canald.cpp
//
// This file is part of the VSCP Project (http://www.vscp.org)
//
// Copyright (C) 2011 Thomas Schulz <tschulz3016@gmail.com>
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



import os
import sys
import time
import json
import logging
from optparse import OptionParser


import vscpdif
from vscp_class import *
#todo: from vscp_type import *


STOP = False
options = None
args = None


def print_event(response_str):

    ev = vscpdif.vscp_rx_event(response_str)
    print ev, "\t", ev.guid


def get_event():

    vscpdif.TN.write("retr\n")
    status, response = vscpdif.get_response(timeout=1)
    if status:
        return response
    return None


logger = None

def init_logger():
    global logger
    logger = logging.getLogger(NAME)
    logger.setLevel(logging.DEBUG)
    ch = logging.StreamHandler()
    ch.setLevel(logging.DEBUG)
    logger.addHandler(ch)


def run():

    if REGISTER[NICKNAME] == 0xff:
        discover_nickname()

    logger.info("Going online. Nickname: %d" % (REGISTER[NICKNAME]))
    send_probe(REGISTER[NICKNAME])

    while not STOP:

        ev = get_event()
        if ev:
            #print_event(ev)
            process_event(ev)
        else:
            time.sleep(0.1)

        if REGISTER[NICKNAME] == 0xff:
            discover_nickname()


def main():
    global options, args

    parser = OptionParser()
    parser.add_option("-i", "--init",
                        action="store_true", dest="init",
                        help="Initialize all registers with 0 (zero).")
    parser.add_option("-n", "--no_store",
                        action="store_true", dest="no_store",
                        help="Don't store changed register values on disk.")
    options, args = parser.parse_args()

    init_logger()

    if options.init:
        registers_init()
        registers_write()
    else:
        registers_read()

    if not vscpdif.open():
        logger.error("vscpdif.open failed")
        return

    logger.info("Connected to vscpd.")
    run()


NICKNAME = 145
REGISTER = {}

NAME = os.path.basename(sys.argv[0])
REGISTER_FILENAME = "%s.registers" % (NAME)


def registers_init():
    for i in range(256):
        REGISTER[i] = 0
    REGISTER[NICKNAME] = 0xff


def registers_read():
    """Read from persistent storage."""

    try:
        register = json.load(file(REGISTER_FILENAME, "r"))
    except IOError:
        logger.warning("No register file found. Initializing default registers.")
        registers_init()
        return

    # Json stores the dictionary keys as strings. I prefer the keys to be integers...
    for k,v in register.items():
        REGISTER[int(k)] = v


def registers_write():
    """Write to persistent storage."""
    if not options.no_store:
        json.dump(REGISTER, file(REGISTER_FILENAME, "w"), indent=0)


def process_event(response_str):
    ev = vscpdif.vscp_rx_event(response_str)

    if ev.class1 == VSCP_CLASS1_PROTOCOL:     # Protocol event

        if ev.type == 2:    # Probe
            if ev.data[0] != REGISTER[NICKNAME]:
                return
            logger.info("I'm being probed!")
            vscpdif.send(class1=VSCP_CLASS1_PROTOCOL, type=3)   # Probe ACK
            return

        if ev.type == 6:    # Set nickname
            if ev.data[0] != REGISTER[NICKNAME]:
                return
            REGISTER[NICKNAME] = ev.data[1]
            vscpdif.send(class1=VSCP_CLASS1_PROTOCOL, type=7)   # Nickname id accepted
            logger.info("Set nickname to: %d" % (ev.data[1]))
            registers_write()
            return

        if ev.type == 8:    # Drop nickname
            if ev.data[0] != REGISTER[NICKNAME]:
                return
            REGISTER[NICKNAME] = 0xff
            logger.info("Drop nickname")
            discover_nickname()
            registers_write()
            return

        if ev.type == 9:    # Read register
            if ev.data[0] != REGISTER[NICKNAME]:
                return
            reg = ev.data[1]
            val = REGISTER[reg]
            vscpdif.send(class1=VSCP_CLASS1_PROTOCOL, type=10, data=[reg, val]) # Read/Write response
            logger.info("Read register(%d) => %d" % (reg, val))
            return

        if ev.type == 11:   # Write register
            if ev.data[0] != REGISTER[NICKNAME]:
                return
            reg = ev.data[1]
            val = ev.data[2]
            REGISTER[reg] = val

            vscpdif.send(class1=VSCP_CLASS1_PROTOCOL, type=10, data=[reg, REGISTER[reg]])   # Read/Write response
            registers_write()
            logger.info("Write register(%d) = %d" % (reg, val))
            return

        if ev.type == 12:   # Enter boot loader mode
            if ev.data[0] != REGISTER[NICKNAME]:
                return
            vscpdif.send(class1=VSCP_CLASS1_PROTOCOL, type=14, data=[]) # NACK boot loader mode
            logger.info("Enter boot load mode: not implemented")
            return

        if ev.type == 24:   # Page read
            if ev.data[0] != REGISTER[NICKNAME]:
                return
            logger.warning("Page read: not implemented")

        if ev.type == 31:   # Who is there?
            if ev.data[0] != 0xff:
                if ev.data[0] != REGISTER[NICKNAME]:
                    return
            logger.info("Who is there?")
            for i in range(7):
                row = i * 7
                if i == 6:
                    data = [i, REGISTER[208+row], REGISTER[209+row], REGISTER[210+row], REGISTER[211+row], REGISTER[212+row], REGISTER[213+row]]
                else:
                    data = [i, REGISTER[208+row], REGISTER[209+row], REGISTER[210+row], REGISTER[211+row], REGISTER[212+row], REGISTER[213+row], REGISTER[214+row]]
                vscpdif.send(class1=VSCP_CLASS1_PROTOCOL, type=32, data=data)
            return


def send_probe(target):
    vscpdif.send(class1=VSCP_CLASS1_PROTOCOL, type=2, data=[target])   # New node on line / Probe


def discover_nickname():

    for i in range(1,255):
        nn_in_use = False
        for j in range(5):
            logger.debug("discover nickname %d/%d" % (i,j))
            send_probe(i)
            response = vscpdif.get_response_match(class1=VSCP_CLASS1_PROTOCOL, type=3)
            if response:
                # Nickname in use already. Don't probe this one again.
                nn_in_use = True
                break

        if nn_in_use:
            # Try next one.
            continue

        # Not in use, so it is available
        send_probe(i)
        REGISTER[NICKNAME] = i
        registers_write()
        logger.info("I'm node: %d", i)
        break


if __name__ == "__main__":
    main()
