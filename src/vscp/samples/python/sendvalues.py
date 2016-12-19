#!/usr/bin/python

import getpass
import sys
import telnetlib

if ( len(sys.argv) < 4 ):
	sys.exit("Wrong number of parameters - aborting")

event = "0,20,3,0,0,0:1:2:3:4:5:6:7:8:9:10:11:12:13:14:15,0,1,35"
#HOST = "192.168.1.6"
#user = raw_input("Enter your remote account: ")
#password = getpass.getpass()

host = sys.argv[1]
user = sys.argv[2]
password = sys.argv[3]

tn = telnetlib.Telnet(host, 9598)
tn.read_until("+OK",2)

tn.write("user " + user + "\n")
tn.read_until("+OK", 2)

tn.write("pass " + password + "\n")
tn.read_until("+OK - Success.",2)

tn.write("send " + event + "\n")
tn.read_until("+OK - Success.",2)

tn.write("quit\n")


