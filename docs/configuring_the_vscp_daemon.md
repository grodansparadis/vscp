# Configuring the VSCP Daemon

## Location of the configuration file

The daemon needs a configuration file called **vscpd.conf**. It will not start without this file nor if the file contains invalid information. Normally the installation program will create a default file.

On most machines it will be located in 

    /etc/vscp
    
The location that is searched to find the configuration file can be changed with switches when you start the daemon. see the section about [startup switches](http://www.vscp.org/docs/vscpd/doku.php?id=vscp_daemon_startup_switches) for each platform.

The configuration file have a path to a sql database file that store configuration setting among other things. Many configuration items are possible to set in both the xml file and in sql file. If a setting is present in both the setting in the xml file will be used. 

##  Description of the configuration :id=config-description

The configuration file is a standard XML file that contains information that tells the VSCP daemon what to do and how it should be done. The information in it is divided into sections and this documentation and each section and it's content is described below.

You can view a sample configuration file [here](Sample vscpd configuration file).

## The general section :id=config-general

In the general section you find settings that are common to all components of the VSCP daemon software. 

```xml
<general runasuser="vscp"
    guid="FF:FF:FF:FF:FF:FF:FF:F5:00:00:00:00:00:00:00:01"
    debug=""
    servername="The VSCP daemon" />
```


### runasuser :id=config-general-runasuser

__Only on Unix/Linux__. User to run the VSCP daemon as.



### guid :id=config-gerneral-guid

```xml
<guid>
    FF:FF:FF:FF:FF:FF:FF:FF:FF:FF:FF:F5:00:00:00:00
</guid>
```

Set the server GUID for the daemon. This GUID is the base for all interface GUID's in the system. Interface GUID's is formed like xx:xx:xx:xx:xx:xx:xx:xx:xx:xx:xx:xx:aa:aa:bb:bb where xx is guid set here, aa is the interface id and bb is nickname id's for devices on that interface. 

If not set here (or all nills) a GUID will be formed from the (first) MAC address of the machine the daemon runs on (on Linux needs to be run as root to get this) or if this fails the local IP address is used to form the GUID. The GUID string should be in MSB -> LSB order and have the four lsb set to zero.

Clients will be identified with a GUID that consist of this GUID plus an on the system unique number in the aa aa bytes. Devices will use this GUID as the source for events if no GUID is set for the driver or if it is set to all zeros.

**Example**
    FF:EE:DD:CC:BB:AA:99:88:77:66:55:44:00:00:00:00

### debug :id=config-gerneral-debug    

----

### Servername :id=config-general-servername

If set this real text name will be used as an identifier for the server along with the GUID. The default name will be something like

    VSCP Daemon @ FF:FF:FF:FF:FF:FF:FF:FE:00:26:55:CA:1F:DA:00:00


----
##  Level I Drivers :id=config-level1-driver

Enable/disable daemon Level I driver interface. If disabled no Level I drivers will be loaded. Default is enabled. This is the driver type that in the past used to be called CANAL (CAN Abstraction Layer) drivers.

```xml
<level1driver enable=”true|false” />
```

The content consist of one or several driver entries each representing a level 1 driver that should be used. 

### enable

Set to true to enable level I driver functionality , set to false to disable. This is convenient if one wants to disable all level I drivers without removing them from the configuration file.

### driver

```xml
<driver enable="false">
```

A level I driver entry.

### enable

The driver should be loaded if set to true. If set to false the driver will not be loaded.

### name

```xml
<name>driver-name</name>
```

A name given by the user for the driver. This is the name by which the driver is identified by the system.

### config

```xml
<config>config1;config2</config>
```

The semicolon separated configuration string for the driver. The meaning of this string is driver specific.

### flags

```xml
<flags>0</flags>
```

A 32 bit driver specific number that have bits set that have special meaning for a driver. See the documentation for a specific driver for an explanation of the meaning of each flag bit for that driver.

### path

```xml
<path>path-to-driver</path>
```

Path to where the driver is located. A dll file on Windows or a .so dl file on Linux etc.

### guid

```xml
<guid>guid-for-driver</guid>
```

The GUID that the driver will use for it's interface. This means that this is the GUID that will be the source of Level I events but only if the GUID is not set to all zeros in which case the generated interface GUID from the general section will be used as the base for the GUID. 

Note that the GUID set here will have the two least significant bytes replaced by the node id so when set here they should be set to zero.

### Translation

`<translation>` is a list of semicolon separated fields with translations that should be carried out by the system on event passing through driver.

 | Mnemonic  | Description  | 
 | :--------:  | -----------  | 
 | Bit 1 (1) | Incoming Level I measurement events is translated to Level II measurement, floating point events. | 
 | Bit 2 (2) | Incoming Level I measurement events is translated to Level II measurement, string events.  | 
 | Bit 3 (4) | All incoming events will be translated to class Level I events over Level II, that is have 512 added to it's class. | 

** Example **

```xml
<leve1driver enable="true" >

    <!-- The level I logger driver  -->
    <driver enable="false">
        <name>logger</name>
        <config>/tmp/canallog.txt</config>
        <path>/ust/local/lib/canallogger.so</path>
        <flags>1</flags>
        <guid>00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00</guid>
        <translate>2</translate>
    </driver>

    <!-- The can232 driver -->
    <driver enable="false" >
        <name>can232</name>
        <config>/dev/ttyS0;19200;0;0;125</config>
        <path>/usr/local/lib/can232drv.so</path>
        <guid>00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00</guid>
        <flags>0</flags>
    </driver>

    <!-- The xap driver is documented  -->
    <driver enable="false" >
        <name>xap</name>
        <config>9598;3639</config>
        <path>/usr/local/lib/xapdrv.so</path>
        <flags>0</flags>
        <guid>00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00</guid>
    </driver>

</leve1driver>
```

----

## Level II drivers :id=config-level2-driver

Level II drivers can handle the full VSCP abstraction and don't have the limitation of the Level I drivers. 


```xml
<leve2driver enable=”true|false” />
```

Enable/disable daemon Level II driver interface. If disabled no Level II drivers will be loaded. Default is enabled. 


```xml
`<driver enable="false">`
```

The content consist of one or several driver entries each representing a driver that should be used. 

### enable

The driver should be loaded if set to true. If false the driver will not be loaded.

### name

```xml
`<name>`driver-name`</name>`
```

A name given by the user for the driver.

### config

```xml
`<config>`config1;config2`</config>`
```

The semi colon separated configuration string for the driver. The meaning of this string is driver specific.

###  guid

```xml
`<guid>`guid-for-driver`</guid>`
```

The GUID that the driver will use for it's interface.

### translation

See explanation for Level I drivers above.

### known-nodes

See explanation for Level I drivers above.

**Example**

```xml
<level2driver enable="true|false">

    <driver enable="true|false" >
        <name>test</name>		
        <path>path_to_driver`</path>
        <config>param2;param2;param3;...</config>
        <guid>00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00</guid>
        <known-nodes>
            <node   guid="00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:01" name="Known node1" />
            <node guid="00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:02" name="Known node2" />
            <node guid="00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:03" name="Known node3" />
            <node guid="00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:04" name="Known node4" />
        </known-nodes>
    </driver>

</level2driver>
```


[filename](./bottom_copyright.md ':include')
