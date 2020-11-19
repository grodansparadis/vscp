# Lawicel CANUSB Level I Driver

**Available for:** Windows. Use socketcan on Linux.

This is a device driver for the [Lawicel CANUSB adapter](https://www.canusb.com). It is the property of Lawicel AB and source is therefore not available. The driver can be downloaded from the above site or requested from [Lawicel AB](https://www.lawicel.se). 

**Important!**

Note that before you use this driver the ftdi dxx libraries must be installed. Information about how to install them is available on the CANUSB site or on the [ftdi site](https://www.ftdichip.com). We recommend using the combined VCI + DXX driver which also is available on the CANUSB site in the download section. 

## Parameter String

The driver string has the following format (note that all values can be entered in either decimal or hexadecimal form (for hex precede with 0x).

   adapter-code;bus-speed;filter;mask

To use default values just skip parameters. 

##### adapter-code

adapter-code is the only mandatory but can be set to “NULL” to use the first found adapter on the system. To find the adapter-code use the adaptercheck.exe program that is part of the CANUSB installation. A typical looking adapter code is “LWNQ06ES”. 

##### bus_speed

This is the speed for the CAN bus. It can be given as


*  5 for 5 Kbps 
*  10 for 10 Kbps 
*  20 for 20 Kbps 
*  50 for 50 Kbps 
*  100 for 100 Kbps
*  125 for 125 Kbps 
*  250 for 250 Kbps 
*  500 for 500 Kbps 
*  800 for 800 Kbps 
*  1000 for 1000 Mbs

##### filter

Is the hardware dependent filter for this board hardware. Note that this filter work in a different way than the CANAL filter. Check the CANUSB documentation.

    0x00000000 
    
is the default which receives all messages. 

##### mask

Is the hardware dependent mask for this board hardware. Note that this filter work in a different way than the CANAL filter. Check the CANUSB documentation.

0xFFFFFFFF 

is the default  which receives all messages. 

## Example

For the CANUSB adapter use

    LWNQ06ES;125

which says to use the CANUSB adapter with adaper-ID LWNQ06ES and 125 kbps. You get the same result with

    LWNQ06ES

which will use 125 kbps as default. 

## Flags

##### bit 0

CANUSB_FLAG_TIMESTAMP - Timestamp will be set by adapter. If not set timestamp will be set by the driver. 

##### bit 1

CANUSB_FLAG_QUEUE_REPLACE Normally when the input queue is full new messages received are disregarded by setting this flag the first message is the queue is removed to make room for the new message. 

##### bit 2

CANUSB_FLAG_BLOCK Can be set to make Read and Writes blocking.

**Important!** This bit must be set if the CANAL blocking methods should be used. 

##### bit 3

CANUSB_FLAG_SLOW This flag can be used at slower transmission speeds where the data stream still can be high. Every effort is made to transfer the frame even if the adapter reports that the internal buffer is full. Normally a frame is trashed if the hardware buffer becomes full to promote speed. 

##  Status return 

The CanalGetStatus call returns the status structure with the channel_status member having the following meaning:


*  Bit 0-15 CANUSB Adapter specific. 

*  Bit 16-31 CANAL specified.


[filename](./bottom_copyright.md ':include')
