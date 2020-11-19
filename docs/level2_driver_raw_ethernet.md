# Raw Ethernet level II driver

 Available for: **Linux**, **Windows**
 
This driver gives support for nodes that communicate using the [VSCP raw Ethernet format](http://docs.vscp.org/spec/latest/#/./vscp_over_ethernet_raw_ethernet) and it makes it possible to build nodes that use the vscp protocol and behave like high level nodes without the extra burden of a TCP/IP stack. 

To use this driver on Windows the [winpcap](https://www.winpcap.org) package must be installed.  To use it on Linux the daemon must be run as root and you need the **libpaps-dev** package installed.

To use this driver on Linux the daemon must be run with root privileges.

**Driver Linux:** vscpl2_rawethernetdrv.so  
**Driver Windows:** vscpl2_rawethernetdrv.dll 

The **configuration string** have the following format

    device;localmac[;filter;mask;subaddr]

##### Device

This is a string that identifies the Ethernet device that should be used when sending/receiving Ethernet frames. Typically it's in the form of *eth0*, *eth1*, *eth2* etc on Linux devices and *\Device\NPF_{986752B0-3C0E-46A2-AFD3-B593E180EC54* on Windows. The tool **iflist** that is distributed with VSCP & friends on Windows can be used to list available interfaces on a specific windows system. 

##### local-mac

This is the mac address used as the outgoing mac address when sending raw Ethernet frames. Normally this should be the same as for the adapter. The form i typically *00:26:55:CA:1F:DA*. On windows you can get the MAC address with the *ipconfig /all* command and on Linux with the *ifconfig* command.

##### Variables fetched from the VSCP demon configuration file

 | Variable name  | Type   | Description  | 
 | -------------  | ----   | -----------  | 
 | **_interface** | string | A string that identifies the Ethernet device that should be used when sending/receiving Ethernet frames. Typically it's in the form *eth0*, *eth1*, *eth2* etc on Linux devices and *\Device\NPF_{986752B0-3C0E-46A2-AFD3-B593E180EC54* on a Windows system. | 
 | **_localmac**  | string | The mac address used as the outgoing mac address when sending raw Ethernet frames. Normally this should be the same as for the adapter. The form i typically *00:26:55:CA:1F:DA*.                                                                                  | 
 | _filter        | string | Standard VSCP filter in string from. *1,0x0000,0x0006,ff:ff:ff:ff:ff:ff:ff:01:00:00:00:00:00:00:00:00* as // priority,class,type,GUID// Used to filter what is sent from VSCP out on Ethernet.                                                                     | 
 | _mask          | string | Standard VSCP mask in string form. *1,0x0000,0x0006,ff:ff:ff:ff:ff:ff:ff:01:00:00:00:00:00:00:00:00* as *priority,class,type,GUID* Used to filter what is sent from VSCP out on Ethernet.                                                                        | 
 | **_subaddr**   | long   | This can be a value between 0x0000 - 0xfff (defaults to 0x0000) and is the two least significant bits of the GUID for a transmitted fram.                                                                                                                            | 
The full variable name is built from the name you give the driver (prefix before _variablename) in vscpd.conf. So in the examples below the driver have the name **raweth1** and the full variable name for the **_interface** will thus be

    raweth1_interface

If you have another diver and name it  **raweth2** it will therefore instead request variable **raweth2_interface**

If your driver name contains spaces, for example “name of driver” it will get a prefix that is “name_of_driver”. Leading and trailing spaces will be removed. 

A typical configuration example settings for a Windows 7 machines *vscpd.conf* configuration file is 

```xml
<!-- Level II raw ethernet  -->
<driver enable="true">   	 			
    <name>rawethernet1</name>
    <config>\Device\NPF_{DE579129-5F36-4507-A416-C2B693AC3018};00:26:55:CA:1F:DA</config>
    <path>C:\Program Files (x86)\VSCP\drivers\level2\vscpl2_rawethernetdrv.dll</path>
</driver>
```
        
and on a Linux machine        

```xml

<driver enable="true">
    <name>raweth1</name>
    <config>eth0;00:26:55:CA:1F:DA</config>
    <path>/usr/local/lib/vscp2drv_raweth.so</path>
</driver>
```


#####  Frame format

The full description of the VSCP raw ethernet format is [here](http://docs.vscp.org/spec/latest/#/./vscp_over_ethernet_raw_ethernet).

##### Client support

For Arduino devices you can find a library [here](http://www.airspayce.com/mikem/arduino/EtherRaw/index.html). There is some sample code [here](https://forum.arduino.cc/index.php?topic=74547.0).

[filename](./bottom_copyright.md ':include')
