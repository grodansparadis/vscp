# VSCP Daemon Level I Drivers

This driver type is also called CANAL (CAN Abstraction Layer) drivers after the driver API they use. Most of them have no relation to [CAN](https://sv.wikipedia.org/wiki/Controller_Area_Network) other than the CANAL API which is used as a least common denominator allowing this type of driver handle many types of busses and devices.

The driver is just a .dll (Windows) or a .so (Linux) file with the Level I/CANAL interface exported. The full interface is described [here](https://docs.vscp.org/canal/latest/#/). 

Many ready to use Level I drivers are available in the VSCP & Friends package. A list of drivers can be found [here](https://docs.vscp.org/#level1drv)

The good thing with the Level I interface is that you can add the .dll or .so as a driver for use with the VSCP daemon or use the dll/do directly from your own application such as in [VSCP Works](https://docs.vscp.org/#vscpworks).

The drivers are configured in the vscpd.json file (format is [here](./configuring_the_vscp_daemon.md)). 

To make your own Level I driver just create a dynamically linked library that export the CANAL interface. There are plenty of examples to use as a starting point for creating your own driver in the [source tree for the VSCP & Friends package at GitHub](https://github.com/grodansparadis?tab=repositories&q=vscpl1drv&type=&language=). 

For Python developers [python-can](https://python-can.readthedocs.io/en/master/) is a good tool. __Unfortunately__ the CANAL interface is named USB2CAN here but it is there.

 
[filename](./bottom_copyright.md ':include')
