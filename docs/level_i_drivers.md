# VSCP Daemon Level I Drivers

This driver type is also called CANAL (CAN Abstraction Layer) drivers after the driver API they use. Most of them have no relation to [CAN](https://sv.wikipedia.org/wiki/Controller_Area_Network) other than the CANAL API whcigh is used as a least common denominator allowing this type of driver handle many types of busses and devices.

The driver is just a .dll (Windows) or a .so (Linux) file with the Level I/CANAL interface exported. The fullinterface is described [here](https://docs.vscp.org/canal/latest/#/). 

Many ready to use Level I drivers are available in the VSCP & Friends package. A complete list of drivers can be found [here](https://github.com/grodansparadis?utf8=%E2%9C%93&tab=repositories&q=vscpl1drv-&type=&language=)

The good thing with the Level I interface is that you can add the .dll or .so as a driver for use with the VSCP daemon or use the dll/do directly from your own application such as in VSCP Works.

The drivers are configured in the vscpd.conf file (format is [here](http://www.vscp.org/docs/vscpd/doku.php?id=configuring_the_vscp_daemon)). 

To make your own Level I driver just create a dynamically linked library that export the CANAL interface. There are plenty of examples to use as a starting point for creating your own driver in the [source tree for the VSCP & Friends package at GitHub](https://github.com/grodansparadis?tab=repositories&q=vscpl1drv&type=&language=). 

For Python developers [python-can](https://python-can.readthedocs.io/en/master/) is a good tool. __Unfortunately__ the CANAL interface is named USB2CAN but it is there.


*  [8Devices USB2CAN Driver](https://github.com/grodansparadis/vscpl1drv-usb2can)

*  [Apox Driver](https://github.com/grodansparadis/vscpl1drv-apox)

*  [CAN4VSCP Driver](https://github.com/grodansparadis/vscpl1drv-can4vscp)

*  [CCS CAN Driver](https://github.com/grodansparadis/vscpl1drv-ccs)

*  [IXATT VCI Driver](https://github.com/grodansparadis/vscpl1drv-ixxatvc2)

*  [Lawicel CAN232 Driver](https://github.com/grodansparadis/vscpl1drv-can232)

*  [Lawicel CANUSB Driver](https://www.can232.com/?page_id=75)

*  [LIRC Driver](https://github.com/grodansparadis/vscpl1drv-lirc)

*  [Logger Driver](https://github.com/grodansparadis/vscpl1drv-logger)

*  [PEAK CAN Adapter Driver](https://github.com/grodansparadis/vscpl1drv-peak)

*  [Socketcan Driver](https://github.com/grodansparadis/vscpl1drv-socketcan)

*  [Tellstick Driver](https://github.com/grodansparadis/vscpl1drv-tellstick)

*  [Vector CAN Driver](https://github.com/grodansparadis/vscpl1drv-vector)

*  [Zanthic CAN Driver](https://github.com/grodansparadis/vscpl1drv-zanthic)
 
[filename](./bottom_copyright.md ':include')
