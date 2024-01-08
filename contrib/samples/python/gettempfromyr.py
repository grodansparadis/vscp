#!/usr/bin/python
#!usr/bin/env python

"""
// File: gettempfromyr.py
//
// Usage: gettempfromyr
//
// Described here file:///home/akhe/development/sendvalues.py 
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
//
// This file is part of the VSCP (https://www.vscp.org)
//
// Copyright (C) 2000-2024 Ake Hedman,
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

import socket
import sys
import urllib
import telnetlib

host = "192.168.1.6"
port = 9598
user = "admin"
password = "secret"

guid = "FF:FF:FF:FF:FF:FF:FF:FE:B8:27:EB:40:59:96:00:03"

debug = 0
usesockets = 0
url = "http://tinyurl.com/temperaturLos" #Url to yr.no:s rss-feed
opener = urllib.FancyURLopener({})
page = opener.open(url)

for line in page.readlines():
    if "m/s" in line:
	line = line.split(".")
        strtemp = line[1][:-5].strip()
        wind_list = line[3].split(",")        
        wind_list = wind_list[1].split("fra")
        strwind = wind_list[0][:-4].strip() 
        break
			
# debug output		
if (debug):
    print "Temperature = " + strtemp + " degrees Celsius"
    print "Wind = " + strwind + " m/s"

if ( usesockets ):
    
    temp = int(strtemp)
    wind = int(strwind)

    # create socket
    s = socket.socket( socket.AF_INET, socket.SOCK_STREAM )

    # connect to server 
    
    s.connect( ( host, port ) )
    data = s.recv( 10000 )
    if (1 == debug):
        print data

    s.send( "user " + user + "\r\n" )
    data = s.recv( 10000 )
    if (1 == debug):
        print data

    s.send( "pass " + password + "\r\n" )
    data = s.recv( 10000 )
    if (1 == debug):
        print data

    # Send temperature forecast
    s.send( "SEND 0,10,6,,0,0," + guid + ",0x88,0," + str( temp ) + "\r\n" )
    data = s.recv( 10000 )
    if (1 == debug):
        print data

    # Send wind forecast CLASS.MEASUREMENT, Type=Speed
    # coding = 0b100 =  Normalized integer
    # unit = 0 = m/s
    # sensor = 0
    s.send( "SEND 0,10,32,,0,0," + guid +",0x80,0," + str( wind ) + "\r\n" )
    data = s.recv( 10000 )
    if (1 == debug):
        print data

    # close connection
    s.close();

# telnet lib
else:

    # Connect to VSCP daemon
    tn = telnetlib.Telnet(host, 9598)
    tn.read_until("+OK - Success.",2)

    # Login
    tn.write("user " + user + "\n")
    tn.read_until("+OK - Success.", 2)

    tn.write("pass " + password + "\n")
    tn.read_until("+OK - Success.",2)

    # *******************************************
    #            Temperature forecast
    # *******************************************

    event = "3,"	    # Priority=normal
    event += "10,6,"	    # Temperature measurement class=10, type=6
    event += ","	    # DataTime
    event += "0,"	    # Use interface timestamp
    event += "0,"  	    # Use obid of interface
    event += guid + ","
    # datacoding = String format| Unit = Celsius | sensor 0
    datacoding = 0x40 | (1<<3) | 0  
    event += hex(datacoding)	# Add datacoding byte to event
    
    # Make sure length is OK (max seven characters)
    strtemp = strtemp.strip()
    if ( len(strtemp) > 7 ):
        strtemp = strtemp[0:7]

    # Write temperature into the event (not line breaks)
    for ch in strtemp:
        if  ( ( 0x0a != ord(ch) ) and ( 0x0d != ord(ch) ) ):
            event += ","
            event += hex(ord(ch))

    if ( debug ): print "Temperature event " + event        
    tn.write( "SEND " + event + "\n" )
    rv = tn.read_until("+OK - Success.", 2)
    if ( debug ): print rv

    # *******************************************
    #                 Temp Warning
    # *******************************************

    event = "3,"	    # Priority=normal
    event += "95,"	    # Weather forecast
    if ( int( strtemp ) ) <= -20:
        event += "11,"   # "Freezing"
    elif ( int( strtemp ) <= -15 ):
        event += "12,"   # "Very cold" 
    elif ( int( strtemp ) <= 0 ):
        event += "13,"   # "Cold"
    elif ( int( strtemp ) < 20 ):
        event += "14,"   # "Normal"
    elif ( int( strtemp ) < 30 ):
        event += "15,"   # "Hot"    
    else :
        event += "16,"   # "Very hot"    

    event += ","         # Datettime
    event += "0,"  	 # Use timestamp of interface
    event += "0,"        # Use obid of interface
    event += guid + ","
    event += "0,"	 # IndexError
    event += "0xff,"	# Zone - all
    event += "0xff,"	# Subzone - all

    if ( debug ): print "Wind speed forecast " + event            
    tn.write( "SEND " + event + "\n" )
    tn.read_until("+OK - Success.", 2)

    # *******************************************
    #               Wind forecast
    # *******************************************

    event = "3,"	# Priority=normal
    event += "10,32,"	# Speed measurement class=10, type=6
    event += ","        # DateTime
    event += "0,"  	# timestamp
    event += "0,"       # Use obid of interface
    event += guid + ","
    # datacoding = String format| unit = m/s | sensor = 0
    datacoding = 0x40 | 0 | 0  
    event += hex(datacoding)	# Add datacoding byte to event
    
    # Make sure length is OK (max seven characters)
    strwind = strwind.strip()
    if ( len(strwind) > 7 ):
        strwind = strwind[0:7]

    # Write temperature into the event (not line breaks)
    for ch in strwind:
        if  ( ( 0x0a != ord(ch) ) and ( 0x0d != ord(ch) ) ):
            event += ","
            event += hex(ord(ch))

    if ( debug ): print "Wind speed event " + event            
    tn.write( "SEND " + event + "\n" )
    tn.read_until("+OK - Success.", 2)

    # *******************************************
    #                 Wind Warning
    # *******************************************

    event = "3,"	    # Priority=normal
    event += "95,"	    # Weather forecast
    if ( int( strtemp ) ) == 0:
        event += "5,"   # "No wind"
    elif ( int( strtemp ) <= 5 ):
        event += "6,"   # "Low wind" 
    elif ( int( strtemp ) <= 10 ):
        event += "7,"   # "Medium wind"
    elif ( int( strtemp ) <= 15 ):
        event += "8,"   # "High wind"
    else :
        event += "9,"   # "Very high wind"    
    event += ","        # DateTime
    event += "0,"       # Timestamp
    event += "0,"  	# Use obid of interface
    event += guid + ","
    event += "0,"	# IndexError
    event += "0xff,"	# Zone - all
    event += "0xff,"	# Subzone - all

    if ( debug ): print "Wind speed forecast " + event            
    tn.write( "SEND " + event + "\n" )
    tn.read_until("+OK - Success.", 2)

    tn.write("quit\n")    

