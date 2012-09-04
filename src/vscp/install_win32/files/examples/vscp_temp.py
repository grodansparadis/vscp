#!usr/bin/env python
import socket
import sys
import urllib

url = "http://tinyurl.com/temperaturLos" #Url till yr.no:s rss-feed
opener = urllib.FancyURLopener({})
page = opener.open(url)

for line in page.readlines():
    if "m/s" in line:
	line = line.split(".")
        temperatur = line[1][:-5].strip()
        vindstyrka_list = line[3].split(",")        
        vindstyrka_list = vindstyrka_list[1].split("fra")
        vindstyrka = vindstyrka_list[0][:-4].strip() 
        break
		
temp = int(temperatur)
wind = int(vindstyrka)
		
# debug output		
print "Temperature = " + temperatur + " grader celsius"
print "Vindstyrka = " + vindstyrka + " m/s"



# create socket
s = socket.socket( socket.AF_INET, socket.SOCK_STREAM )

# connect to server
host = "dosilos.se"
port = 9598
s.connect( ( host, port ) )
data = s.recv( 10000 )
print data

s.send( "user admin\r\n" )
data = s.recv( 10000 )
print data

s.send( "pass secret\r\n" )
data = s.recv( 10000 )
print data

#
# Send a full GUID event
# ======================
# Format is
#      "send head,class,type,obid,timestamp,GUID,data1,data2,data3...."
#
# Class=20  CLASS1.INFORMATION
# TYPE = 3  ON
# obid = 0
# timestamp = 0
# GUID = 0:1:2:3:4:5:6:7:8:9:10:11:12:13:14:15   on the form MSB->LSB
# Zone=1
# SubZone=80
#
s.send( "SEND 0,20,3,0,0,0:1:2:3:4:5:6:7:8:9:10:11:12:13:14:15,0,1,80\r\n" ) 
data = s.recv( 10000 )
print data

#
# The same thing can be sent using
#
# SEND 0,20,3,0,0,-,0,1,80
#
# where the  interface GUID is used.
s.send( "SEND 0,20,3,0,0,-,0,1,80\r\n" )
data = s.recv( 10000 )
print data

# Send temperature forecast
s.send( "SEND 0,10,6,0,0,-,0x88,0," + str( temp ) + "\r\n" )
data = s.recv( 10000 )
print data

# Send wind forecast
s.send( "SEND 0,10,32,0,0,-,0x88,0," + str( wind ) + "\r\n" )
data = s.recv( 10000 )
print data

# close connection
s.close();

