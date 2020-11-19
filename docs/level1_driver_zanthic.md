# Zanthic CAN Level I Driver

**Available for:** Windows

**Driver for Windows** vscpl1_zanthicdrv.dll (vscpl1_zanthicdrv.lib)

This is a device driver for the [Zanthic Technologies](https://www.zanthic.com/products.htm) USB adapter. 

##### Parameter String

The driver string has the following format (note that all values can be entered in either decimal or hexadecimal form (for hex preceded with 0x).

##### bus-speed

This is the speed for the CAN bus. It can be given as 

   * 10 for 10 Kbps 
   * 20 for 20 Kbps 
   * 50 for 50 Kbps 
   * 100 for 100 Kbps 
   * 125 for 125 Kbps
   * 250 for 250 Kbps
   * 500 for 500 Kbps 
   * 800 for 800 Kbps 
   * 1000 for 1000 Mbps

## Flags

Not used set to 0.

## Status return

The CanalGetStatus call returns the status structure with the channel_status member having the following meaning:

   * Bit 0-31 Reserved.

[filename](./bottom_copyright.md ':include')
