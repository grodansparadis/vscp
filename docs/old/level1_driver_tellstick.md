# Tellstick Level I Driver

**Available for:** Windows, Linux

**Driver for windows** vscpl1_tellstickdrv.dll (vscpl1_tellstickdrv.lib)
**Driver for Linux** vscpl1_tellstickdrv.so

This is a driver for the USB module available from [Telldus](https://www.telldus.se). This module can be used to control wireless switches etc from [many vendors](https://www.telldus.se/receivers.html). 

## Parameter string

### Windows

    device-id;path-to-config

##### device-id

This is the ID for the actual Tellstick adapter. If not present the first found adapter will be used.

##### path-to-config

This is the path to the XML config file. If absent the driver will search for the file tellstick.xml in vscp folder the current users application data folder. 

### Linux

    usb-device;device-id;path-to-config

##### usb-device

Physical USB channel e.g. /dev/ttyUSB0. Mandatory.

##### device-id

This is the ID for the actual tellstick adapter. If not present the first found adapter will be used.

##### path-to-config

This is the path to the XML config file. If absent the driver will search for the file tellstick.xml in vscp folder the current users application data folder. 

## Flags

Not used at the moment. 

## Status return

The CanalGetStatus call returns the status structure with the channel_status member having the following meaning:  All bits unused at the moment.

## Configuration file

Configuration for the module is done with a XML file

```xml
<tellstick>
    <!-- Many devies can be set as receivers for one event -->`
    <event type="on|off|dim|all" zone="n" subzone="n">
        <device protocol="NEXA |SARTANO | WAVEMAN | IKEA" 
               systemcode="1-16" `<!-- Needed for IKEA -->			
               devicecode="1-10" <!-- Needed for IKEA -->			
               dimlevel="0-10"	 <!-- If missing and dim event level/10 is used -->
               dimstyle="0|1" 			
               housecode="A-P" <!-- Needed for NEXA, WAVEMAN -->		
			
               channel="1-16 [SARTANO 0-255] " 
               <!-- Needed for NEXA, WAVEMAN and SARTANO -->
			
               state="0|1" 
               <!-- 0=off, 1=on if absent event state is used. -->
               <!-- Needed for NEXA, WAVEMAN, SARTANO -->	
        />		 	
    </event>
</tellstick>
```

The driver understands four events

##### "all"

    CLASS1.CONTROL Class=30, Type=2 ALL LAMPS ON/OFF 

**Reply:** CLASS1.INFORMATION Class=30, Type=3 for ON or Type=4 for OFF for each effected device.

##### "on"

    CLASS1.CONTROL Class=30, Type=5 TURN ON 

**Reply:** CLASS1.INFORMATION Class=30, Type=3 for ON for each effected device. 

##### "off"

    CLASS1.CONTROL Class=30, Type=6 TURN OFF 

**Reply:** CLASS1.INFORMATION Class=30, Type=4 for OFF for each effected device. 

#####  "dim"

    CLASS1.CONTROL Class=30, Type=20 DIM 

**Reply:** CLASS1.INFORMATION Class=30, Type=40 for LEVEL CHANGED for each effected device. 

Replies from the module are not really replies from the controlled nodes themselves as they don't issue a confirm. VSCP devices should ALWAYS reply back there state after a request to change it so here the driver take this responsibility. 


[filename](./bottom_copyright.md ':include')
