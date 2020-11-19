# 8Devices USB2CAN Level I Driver

**Available for:** Windows, Use socketcan on Linux

**Driver for windows** vscpl1_usb2candrv.dll

The complete docs for the driver is [here](http://www.8devices.com/media/files/CANAL_API.pdf)

## Parameter String

    serial_number;baudrate;mask;filter

or if baudrate == 0

    serial_number;baudrate;0;tseg2;tseg1;sjw;brp;mask;filter

note: mask and filter can be omitted

##### serial_numbers

Serial number of the usb2can module. example: *ED0000004* 

##### baudrate

Baudrate for the serial interface 

##### mask

Mask to filter messages.

##### filter

Filter to filter messages.

##### Flags


*  Enable status message 
*  Disable auto retransmission. 
*  Silent. 
*  Loop-back.

## Status return

The CanalGetStatus call returns the status structure with the channel_status member having the following meaning:


*  Bit 0 - TX Error Counter. 
*  Bit 1 - TX Error Counter. 
*  Bit 2 - TX Error Counter. 
*  Bit 3 - TX Error Counter. 
*  Bit 4 - TX Error Counter. 
*  Bit 5 - TX Error Counter. 
*  Bit 6 - TX Error Counter.
*  Bit 7 - TX Error Counter. 
*  Bit 8 - RX Error Counter. 
*  Bit 9 - RX Error Counter. 
*  Bit 10 - RX Error Counter. 
*  Bit 11 - RX Error Counter. 
*  Bit 12 - RX Error Counter. 
*  Bit 13 - RX Error Counter. 
*  Bit 14 - RX Error Counter. 
*  Bit 15 - RX Error Counter. 
*  Bit 16 - Overflow. 
*  Bit 17 - RX Warning. 
*  Bit 18 - TX Warning. 
*  Bit 19 - TX bus passive. 
*  Bit 20 - RX bus passive. 
*  Bit 21 - Reserved. 
*  Bit 22 - Reserved. 
*  Bit 23 - Reserved. 
*  Bit 24 - Reserved. 
*  Bit 25 - Reserved.
*  Bit 26 - Reserved. 
*  Bit 27 - Reserved. 
*  Bit 28 - Reserved. 
*  Bit 29 - Bus Passive. 
*  Bit 30 - Bus Warning status 
*  Bit 31 - Bus off status 

[filename](./bottom_copyright.md ':include')
