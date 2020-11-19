# xAP Level I Driver

----


**This driver is not not available anymore from version 1.1.0**

---- 

**Available for:** Windows, Linux

**Driver for Windows** vscpl1_xapdrv.dll (vscpl1_xapdrv.lib)
**Driver for Linux** vscpl1_xapdrv.so 

This is a driver that make the VSCP/CANAL daemon behaves like a xAP server or client. If no other server is detected when the driver is loaded it will start up in server mode. If a server is detected it will start up in client mode and connect to that server.

As this is a driver only Level I events will be transferred between VSCP and xAP. 

## Parameter String

    uid;port

where

##### uid

is the ID for this device. Use a different ID for every device. Defaults to 9598. 

##### port

Port is the xAP port and defaults to 3639.

## Flags

Not used at the moment. 

##  Status return

Not used at the moment.


\\ 
----
{{  ::copyright.png?600  |}}

`<HTML>``<p style="color:red;text-align:center;">``<a href="http://www.grodansparadis.com">`Grodans Paradis AB`</a>``</p>``</HTML>`
