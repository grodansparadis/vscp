# Socketcan level II driver

**Available for:** Linux

SocketCAN, the official CAN API of the Linux kernel, has been included in the kernel more than 3 years ago. Meanwhile, the official Linux repository has device drivers for all major CAN chipsets used in various architectures and bus types. SocketCAN offers the user a multiuser capable as well as hardware independent socket-based API for CAN based communication and configuration. Socketcan nowadays give access to the major CAN adapters that is available on the market. Note that as CAN only can handle Level I events only events up to class < 1024 can be sent to this device. Other events will be filtered out. Also received events 

**Driver Linux**: vscpl2_socketcandrv.so

The configuration string have the following format

    Interface

##### Interface

The parameter interface is the socketcan interface to use. Typically this is can0, can0, can1... Defaults is vcan0 the first virtual interface. If the variable prefix_interface is available it will be used instead of the configuration value. 

 | Variable name | Type   | Description | 
 | ------------- | ----   | -----------   | 
 | _interface    | string | The socketcan interface to use. Typically this is “can0, can0, can1...” Defaults is vcan0 the first virtual interface. | 
 | _filter       | string | Standard VSCP filter in string form. 1,0x0000,0x0006,ff:ff:ff:ff:ff:ff:ff:01:00:00:00:00:00:00:00:00 as priority,class,type,GUID Used to filter what events that is received from the socketcan interface. If not give all events are received. | 
 | _mask         | string | Standard VSCP mask in string form. 1,0x0000,0x0006,ff:ff:ff:ff:ff:ff:ff:01:00:00:00:00:00:00:00:00 as priority,class,type,GUID Used to filter what events that is received from the socketcan interface. If not give all events are received.   | 

The full variable name is built from the name you give the driver (prefix before _variablename) in vscpd.conf. So in the examples below the driver have the name **socketcan1** and the full variable name for the **_interface** will thus be

    socketcan1_interface

If you have another diver and name it  **socketcan2** it will therefore instead request variable **socketcan2_interface**

If your driver name contains spaces, for example “name of driver” it will get a prefix that is “name_of_driver”. Leading and trailing spaces will be removed. 

##### vscpd.conf example

```xml
<driver enable="true" >
    <name>socketcan1</name>
    <path>/usr/local/lib/vscpl2_socketcandrv.so</path>
    <config>can0</config>
    <guid>00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00</guid>
</driver>
```


## Notes

*  Some notes about socketcan is [here](http://www.akehedman.se/wiki/doku.php/socketcan). 



[filename](./bottom_copyright.md ':include')
