# Introduction to the VSCP Daemon Software

The VSCP daemon is an interface between low level devices and the higher end world. It can also connect to other VSCP servers and hardware nodes forming large and complex systems. The low end devices may be VSCP enabled or other types of devices thant know nothing about VSCP. The driver system of VSCP serve as a hardware abstraction layer that makes it possible to connect to anything that is out there. 

Two sorts of drivers are available, **Level I** and **Level II**, with slightly different capabilities. It is easy to construct new drivers and there are plenty of sample code on how to do so.

From version 15.0 The VSCP daemon needs a MQTT broker but can be still also be used on it's own exporting different interfaces through installed drivers. 


[filename](./bottom_copyright.md ':include')
