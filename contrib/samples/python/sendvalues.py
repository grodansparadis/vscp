#!/usr/bin/python
"""
// File: sendvalue.py
//
// Usage: sendvalue host user password value class type [sensorindex]
//
// Described here file://vscp/src/vscp/samples/python/sendvalues.py
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
//
// This file is part of the VSCP (https://www.vscp.org)
//
// Copyright (C) 2000-2025 Ake Hedman,
// Ake Hedman, the VSCP project, <info@vscp.org>
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

def sendMeasurement10( host, port, user, password, guid, measurement, vcscptype, sensorindex, unit)

    vscpclass = 10  # Measurement class

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
        event = "3,"	# Priority=normal
        event += vscpclass + ","
        event += vscptype + ","
        event += ","	# Use now as DateTime
        event += "0,"	# Use interface timestamp
        event += "0,"  	# Use obid of interface
        event += guid	# add GUID to event

        measurement_str = str(measurement)
        # Make sure length is OK (max seven characters)
        if ( len(measurement_str) > 7 ):
            measurement_str = measurement_str[0:7]

        # datacoding = String format| Celsius | sensor 0
        datacoding = 0x40 | (unit<<3) | sensorindex
        event += hex(datacoding) + ","	# Add datacoding byte to event

        # Write temperature into the event (no line breaks)
        for ch in measurement_str:
            if  ( ( 0x0a != ord(ch) ) and ( 0x0d != ord(ch) ) ):
                event += ","
                event += hex(ord(ch))

        # Send event to server
        tn.write("send " + event + "\n")
        tn.read_until("+OK - Success.",2)

tn.write("quit\n")
