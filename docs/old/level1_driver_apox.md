# Apox Level I Driver

**Available for:** Windows

**Driver for windows** vscpl1_apoxdrv.dll (vscpl1_apoxdrv.lib)

## Parameter String

The driver string has the following format (note that all values can be entered in either decimal or hexadecimal form (for hex preceded with 0x).

    serial;bus-speed

##### serial

This is the serial number for the adapter in use. 

##### buss_speed

This is the speed for the CAN bus. It can be given as 


*  125 for 125Kbps 
*  250 for 250Kbps 
*  500 for 500Kbps 
*  1000 for 1Mbps

## Flags

Not used set to 0. 

## Status return

The CanalGetStatus call returns the status structure with the channel_status member having the following meaning:


*  Bit 0-7   TX error counter. 
*  Bit 8-15  RX error counter. 
*  Bit 16    Overflow. 
*  Bit 17    RX Warning. 
*  Bit 18    TX Warning. 
*  Bit 19    TX bus passive. 
*  Bit 20    RX bus passive. 
*  Bit 21-28 Reserved. 
*  Bit 29    Bus Passive. 
*  Bit 30    Bus Warning status. 
*  Bit 31    Bus off status 



[filename](./bottom_copyright.md ':include')
