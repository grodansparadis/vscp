#!/usr/bin/python
"""
// File: send_pi_gpu_temp.py
//
// Usage: send_pi_gpu_temp host user password [guid]
//
// Described here https://github.com/grodansparadis/vscp/blob/master/src/vscp/samples/python/send_pi_gpu_temp.py
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
//
// This file is part of the VSCP (https://www.vscp.org)
//
// Copyright © 2000-2023 Ake Hedman,
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
// 
"""

import getpass
import sys
import telnetlib
import sys
import os

if ( len(sys.argv) < 4 ):
	sys.exit("Wrong number of parameters - aborting")

guid = "-"
host = sys.argv[1]
user = sys.argv[2]
password = sys.argv[3]
if ( len(sys.argv) > 3 ):
	guid = sys.argv[4]

temperature = os.popen("/opt/vc/bin/vcgencmd measure_temp").read()
temperature = temperature[5:9]
tempfloat = float( temperature )
#sys.exit(tempfloat);


# Connet to VSCP daemon
tn = telnetlib.Telnet(host, 9598)
tn.read_until("+OK",2)

# Login
tn.write("user " + user + "\n")
tn.read_until("+OK", 2)

tn.write("pass " + password + "\n")
tn.read_until("+OK - Success.",2)

event = "3,"		# Priority=normal
event += "10,6,"	# Temperature measurement class=10, type=6
event += ","		# DateTime
event += "0,"		# Use interface timestamp
event += "0,"  		# Use obid of interface
event += guid  + ","	# add GUID to event

# datacoding = String format| Celsius | sensor 0
datacoding = 0x40 | (1<<3) | 0  
event += hex(datacoding)	# Add datacoding byte to event

# Make sure length is OK (max seven characters)
tempstr = str( tempfloat )
tempstr = tempstr.strip()
if ( len(tempstr) > 7 ):
    tempstr = tempstr[0:7]

# Write temperature into the event (not line breaks)
for ch in tempstr:
    event += ","
    event += hex(ord(ch))

# Send event to server
tn.write("send " + event + "\n")
tn.read_until("+OK - Success.",2)

tn.write("quit\n")

