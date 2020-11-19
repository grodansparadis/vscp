# VSCP Daemon Level II Drivers

VSCP drivers or Level II drivers  have two advantages over Level I drivers. 


*  They always use the Level II event format which means that the full GUID is used. 

*  They interface the daemon through the full TCP/IP interface giving a lot of possibilities.

The ability to use the full GUID is good as there is no need for translation schema's between the actual GUID and GUID's used in interfaces where a  node-ID can be abbreviated to eighth or sixteen bits. The full GUI is also globally unique.

Letting the driver talk to the daemon over the TCP/IP interface is favorable in that it can do many things that previously was not possible. The most exciting is that it can read and write variables (even create new ones if needed). This is the recommended way to use for configurations of a Level II driver. It means that configuration of all drivers can be made in one place (the daemon variable file), it also gives a possibility to change runtime values for drivers in real time etc.

The level II driver is, just as the Level I driver, a dynamic link library with a specific set of exported methods. The exported methods are four of the methods from the CANAL interface and uses identical calling parameters and return values. There are some differences however fully described below.

The drivers are configured in the vscpd.conf file (format is [here](./configuring_the_vscp_daemon.md)). If you use more then one driver with different configuration it is very important that the prefix is set to different values for each of them. The prefix is prepended to a variable name before it is fetched from the daemon or set for that matter.

To make a Level II driver just create a dynamically linked library that export the Level II interface. There are plenty of examples to use as a starting point for creating your own driver in the [source tree for the VSCP & Friends package at GitHub](https://github.com/grodansparadis/vscp_software). Look in  [src/vscp/drivers/level2](https://github.com/grodansparadis/vscp_software/tree/master/src/vscp/drivers/level2) folder.

   * [Bluetooth proximity driver](./level2_driver_bluetooth_proximity.md)
   * [LM-sensors driver](./level2_driver_lm_sensors.md)
   * [Logger driver](./level2_driver_logger.md)
   * [MQTT driver](./level2_driver_mqtt.md)
   * [Raw Ethernet driver](./level2_driver_raw_ethernet.md)
   * [Socketcan driver](./level2_driver_socketcan.md)
   * [TCP/IP link driver](./level2_driver_tcpip/link.md)
   * [Linux 1-wire driver](./level2_driver_wire1.md)
   * [Raspberry Pi Linux GPIO driver](./level2_driver_rpigpio.md)
   * [Simulation driver](./level2_driver_simulation.md)


[filename](./bottom_copyright.md ':include')
