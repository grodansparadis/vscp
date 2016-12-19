<h1>Python Scripts and interface to the VSCP daemon</h1>

<b>vscp_class.py</b> This module contains the definitions for the available VSCP 
class id's.

<b>vscp_type.py</b> This module contains the definitions for the available VSCP 
type id's.

<b>vscpdif.py</b> This python module demonstrates how to communicate with the 
vscpd daemon trough the TCP/IP interface. 
Copyright (C) 2009 Thomas Schulz <tschulz@iprimus.com.au>

<b>send burst.py</b> This program has been created to test the data troughput of the daemon by sending a number of messages. 
It is not intended to be bullet proof or to be fit for any purpose. 
Copyright (C) 2009 Thomas Schulz <tschulz@iprimus.com.au>

<b>read_register.py</b> Read registers from a node connected to the vscp daemon.
This module can be used to test the speed of the register update. 
Copyright (C) 2009 Thomas Schulz <tschulz@iprimus.com.au>

<b>write_register.py</b> Read registers from a node connected to the vscp daemon.
This module can be used to test the speed of the register update.
Copyright (C) 2009 Thomas Schulz <tschulz@iprimus.com.au>

<b>read_register.py</b> Read registers from a node connected to the vscp daemon.
This module can be used to test the speed of the register update.
Copyright (C) 2009 Thomas Schulz <tschulz@iprimus.com.au>

<b>page_read.py</b> Read registers from a node connected to the vscp daemon.
This module can be used to test the speed of the register update.
Copyright (C) 2009 Thomas Schulz <tschulz@iprimus.com.au>

<b>client.py</b> A Python VSCP Client.Connects to vscpd via TCP/IP interface.
Implements most important level-1 protocol events. Stores registers in a file.
Copyright (C) 2009 Thomas Schulz <tschulz@iprimus.com.au>

<b>whoisthere.py</b> Check which nodes are connected to the daemon.
Copyright (C) 2009 Thomas Schulz <tschulz@iprimus.com.au>

<b>vscp_temp.py</b> Shows how to feed forecast data from yr.no the the VSCP daemon.

<b>sendvalues.py</b> Script that shows how to take temperature 
sensor values from the one wire digitemp software and send them to the VSCP daemon.
Described here: <a href="www.vscp.org/wiki/doku.php/howto/how_to_setup_digitemp">How to setup digitemp</a>

<b>gettempfromyr.py</p> Weather forecast sample. Reads forcast for temperature and wind 
from yr.no and send to the VSCP daemon.