#!/usr/bin/python

"""
// ORIGINAL from ADAFRUIT (http://www.adafruit.com)
// http://learn.adafruit.com/adafruits-raspberry-pi-lesson-11-ds18b20-temperature-sensing/software
//
// File: send_pi_onewire.py
//
// Usage: ./send_pi_onewire host user password
//
// Described here file:///home/akhe/development/sendvalues.py 
//
// Extensions:
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
//
// This file is part of the VSCP (https://www.vscp.org)
//
// Copyright (C) 2000-2025
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

import os
import sys
import telnetlib
import glob
import time

os.system('modprobe w1-gpio')
os.system('modprobe w1-therm')

onewire_prefix = "FF:FF:FF:FF:FF:FF:FF:FF:"

base_dir = '/sys/bus/w1/devices/'
#base_dir = '/home/akhe/development/simonewire/'   # for test and simulation during development

def construct_guid( line ):
    guid = onewire_prefix + line[0:2].upper() + ":" + \
                line[3:5].upper() + ":" + \
                line[6:8].upper() + ":" + \
                line[9:11].upper() + ":" + \
                line[12:14].upper() + ":" + \
                line[15:17].upper() + ":" + \
                line[18:20].upper() + ":" + \
                line[21:23].upper()
    return guid

def read_temp_raw( device_file ):
    f = open( device_file, 'r' )
    lines = f.readlines()
    f.close()
    return lines

def handle_temperature( device_file ):
    
    # Read lines
    lines = read_temp_raw( device_file )

    # Must be a valid reading
    if ( lines[0].strip()[-3:] != 'YES' ):
        return

    # Find position for temperature data    
    equals_pos = lines[1].find('t=')

    # If temperature data found handle it
    if ( equals_pos != -1 ):
        temp_string = lines[1][ equals_pos + 2:]
        temp_c = float( temp_string ) / 1000.0
        temperature = str( temp_c )

        # construct event
        guid = construct_guid( lines[1] ) 
        event = "3,"	    # Priority=normal
        event += "10,6,"	# Temperature measurement class=10, type=6
        event += "0,"	    # Use interface timestamp
        event += "0,"  	    # Use obid of interface
        event += guid  + ","	# add GUID to event

        # datacoding = String format| Celsius | sensor 0
        datacoding = 0x40 | (1<<3) | 0  
        event += hex(datacoding)	# Add datacoding byte to event

        # Make sure length is OK (max seven characters)
        temperature = temperature.strip()
        if ( len(temperature) > 7 ):
            temperature = temperature[0:7]

        # Write temperature into the event (not line breaks)
        for ch in temperature:
            event += ","
            event += hex(ord(ch))

        # Send event to server
        tn.write("send " + event + "\n")
        tn.read_until("+OK - Success.",2)

if ( len(sys.argv) < 4 ):
    	sys.exit("Wrong number of parameters - aborting")    

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

# Send temperature readings from all sensors
for folder in glob.glob( base_dir + '28*' ):
    device_file =  folder + '/w1_slave'
    handle_temperature( device_file )

# Close connection to VSCP daemon
tn.write("quit\n")



