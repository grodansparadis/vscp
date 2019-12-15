#!/usr/bin/python
"""
// File: sendvalues.py
//
// Usage: digitemp -a -q | send host user password
//
// Described here file:///home/akhe/development/sendvalues.py
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
//
// This file is part of the VSCP (http://www.vscp.org)
//
// Copyright (C) 2000-2019 Ake Hedman,
// Ake Hedman, Grodans Paradis AB, <akhe@vscp.org>
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
//
"""

import getpass
import sys
import telnetlib
import sys

onewire_prefix = "FF:FF:FF:FF:FF:FF:FF:FF:"

if ( len(sys.argv) < 4 ):
	sys.exit("Wrong number of parameters - aborting")

#event = "0,10,6,0,0,0:1:2:3:4:5:6:7:8:9:10:11:12:13:14:15,0,1,35"

host = sys.argv[1]
user = sys.argv[2]
password = sys.argv[3]

# Connect to VSCP daemon
tn = telnetlib.Telnet(host, 9598)
tn.read_until("+OK",2)

# Login
tn.write("user " + user + "\n")
tn.read_until("+OK", 2)

tn.write("pass " + password + "\n")
tn.read_until("+OK - Success.",2)

# For each line from piped digitemp output
for line in sys.stdin:

    guid = onewire_prefix
    event = "3,"	# Priority=normal
    event += "10,6,"# Temperature measurement class=10, type=6
    event += ","	# DateTime
    event += "0,"	# Use interface timestamp
    event += "0,"  	# Use obid of interface

    dtrow = line.split(" ")	# Separate id from temperature
    onewire_id = dtrow[0]	# save sensor id
    temperature = dtrow[1]	# Save temperature reading

    # Reverse temperature id so MSB comes first as VSCP requires
    for i in range(7, -1, -1):
        guid += onewire_id[i*2:i*2+2]
        if ( 0 != i ):
            guid += ":"

    event += guid  + ","	# add GUID to event

    # datacoding = String format| Celsius | sensor 0
    datacoding = 0x40 | (1<<3) | 0
    event += hex(datacoding)	# Add datacoding byte to event

    # Make sure length is OK (max seven characters)
    temperature = temperature.strip()
    if ( len(temperature) > 7 ):
        temperature = temperature[0:7]

    # Write temperature into the event (no line breaks)
    for ch in temperature:
        if  ( ( 0x0a != ord(ch) ) and ( 0x0d != ord(ch) ) ):
            event += ","
            event += hex(ord(ch))

    # Send event to server
    tn.write("send " + event + "\n")
    tn.read_until("+OK - Success.",2)

tn.write("quit\n")
