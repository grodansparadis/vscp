# Lawicel CAN232 Level I Driver

**Available for:** Windows, Linux. Can also use socketcan on Linux.

**Driver for windows:** vscpl1_can232drv.dll (vscpl1_can232drv.lib)  
**Driver for windows:** vscpl1_can232drv.so

This driver interface is for the [can232 adapter from Lawicel](https://www.can232.com). This is a low cost CAN adapter that connects to one of the serial communication ports on a computer. The driver can handle the adapter in both polled and non polled mode, which handled transparently to the user. It is recommended however that the following settings are made before real life use.

   * Set the baud rate for the device to 115200. You do this with the U1 command. This is the default baud rate used by this driver. 
   * Set auto poll mode by issuing the X1 command. 
   * Enable the time stamp by issuing the Z1 command.

## Parameter String

The driver string has the following format (note that all values can be entered in either decimal or hexadecimal form (for hex precede with 0x).

   comport;baudrate;mask;filter;bus-speed;btr0;btr1

##### comport

is the serial communication port to use.(for example 1,2,3….). 

##### baudrate

is a valid baud rate for the serial interface ( for example. 9600 ). 

##### mask

is the mask for the adapter. Read the Lawicel CAN232 manual on how to set this. It is **not** the same as for CANAL/VSCP. 

##### filter

is the filter for the adapter. Read the Lawicel CAN232 manual on how to set this. It is not the same as for CANAL. 

##### bus-speed

is the speed or the CAN interface. Valid values are

   * 10 10Kbps 
   * 20 20Kbps 
   * 50 50Kbps 
   * 100 100Kbps 
   * 125 125Kbps 
   * 250 250Kbps 
   * 500 500Kbps 
   * 800 800Kbps 
   * 1000 1Mbps

##### btr0/btr1

Optional. Instead of setting a bus-speed you can set the SJA1000 BTR0/BTR1 values directly. If both are set the bus_speed parameter is ignored. 

This link can be a help for data [https://www.port.de/engl/canprod/sv_req_form.html](https://www.port.de/engl/canprod/sv_req_form.html)

If no device string is given COM1/ttyS0 will be used. Baud rate will be set to 115200 baud and the filter/mask to fully open. The CAN bit rate will be 500Kbps. 

## Flags

Not used, set to 0. 

## Status return

The CanalGetStatus call returns the status structure with the channel_status member having the following meaning:

   * Bit 0-22  Reserved. 
   * Bit 23    Transmit buffer full. 
   * Bit 24    Receive Buffer Full. 
   * Bit 25-27 Reserved. 
   * Bit 28    Bus Active. 
   * Bit 29    Bus passive status. 
   * Bit 30    Bus Warning status. 
   * Bit 31    Bus off status.

## Example

    5;115200;0;0;1000

Uses COM5 at 115200 with filters/masks open and with 1Mbps CAN bit rate.

   1;57600;0;0;0;0x09;0x1C

Uses COM1 at 57600 with filters/masks open and with bit-rate set to 50Kbps using btr0/btr1


[filename](./bottom_copyright.md ':include')
