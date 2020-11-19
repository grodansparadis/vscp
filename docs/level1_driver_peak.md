# PEAK CAN Adapter Level I Driver

**Available for:** Windows, On Linux use socketcan

**Driver for Windows** vscpl1_peakdrv.dll (vscpl1_peakdrv.lib)

This is a device driver for the [PEAK family of cards](https://www.peak-system.com/fileadmin/media/files/pcan-light.zip). 

The driver uses proprietary include files and libraries from PEAK which are not included in the distribution. 

The driver is included in the installation of the VSCP and friends. Source for the driver is available in the source distribution.

**Important!**

Note that before you use this driver the PEAK original driver dll(s) for your particular adapter(s) must be in the dll search path. These DLL's are delivered from PEAK with your adapter. A good place to install them is in the windows system32 folder. The following table show the dll needed by the CANAL driver for each adapter 

[This is a device driver for the PEAK family of cards](https://www.peak-system.com/fileadmin/media/files/pcan-light.zip) that must have appropriate dll's on your system for the driver to work.


 | Adapter      | PEAK DLL that is needed | 
 | -------      | ----------------------- | 
 | CANDONGLE    | pcan_dng.dll            | 
 | CANDONGLEPRO | pcan_dnp.dll            | 
 | CANISA       | pcan_isa.dll            | 
 | CANPCI       | pcan_pci.dll            | 
 | CANPCI2      | pcan_2pci.dll           | 
 | CANUSB       | pcan_usb.dll            | 

It is important that the PCANBasic.dll file also is in the path or in the same folder as the CANAL driver for everything to work. It can be found [here](https://www.peak-system.com/Support.55.0.html?&L=1).

## Parameter String 

The driver string has the following format (note that all values can be entered in either decimal or hexadecimal form (for hex precede with 0x). 

### For non PnP boards/adapters

    board;bus-speed;hwtype;port;irq;channel;filter;mask 

### For PnP boards/adapters

    board;bus-speed;channel;filter;mask

To use default values just skip parameters. board is the only mandatory parameter. 

##### board

This is one of the following

   * CANDONGLE or 0 for LPT port adapter 
   * CANDONGLEPRO or 1 for LPT port adapter (PRO version) 
   * CANISA or 2 for ISA adapter 
   * CANPCI or 3 for 1 channel PCI adapter 
   * CANPCI2 or 4 for 2 channel PCI adapter 
   * CANUSB or 5 for USB adapter

##### bus_speed

This is the speed for the CAN bus. It can be given as

   * 5 for 5 Kbps 
   * 10 for 10 Kbps 
   * 20 for 20 Kbps 
   * 50 for 50 Kbps 
   * 100 for 100 Kbps 
   * 125 for 125 Kbps 
   * 250 for 250 Kbps 
   * 500 for 500 Kbps 
   * 800 for 800 Kbps 
   * 1000 for 1000 Mbps

##### hwtype

Only valid for non PNP cards 

**CANDONGLE**

   * 2 - dongle
   * 3 - epp 
   * 5 - sja 
   * 6 - sja-epp

**CANDONGLEPRO**

   * 7 - dongle pro 
   * 8 - epp

**CANISA**

   * 1 - ISA 
   * 9 – SJA

##### port

For ISA and parallel port adapters this is the hardware port address 

##### irq

This is the interrupt to use for non PNP devices. 

##### channel

Is a value from 0 an up indicating the CAN channel on the selected board. 

##### filter

Is the hardware dependent filter for this board hardware. Note that this filter may work in a different way than the CANAL filter. 

##### mask

Is the hardware dependent mask for this board hardware. Note that this filter may work in a different way than the CANAL filter. 



## Flags

##### bit 0


*  0 11 bit identifier mode 

*  1 11/29 bit identifier mode (Use for VSCP)

## Status return

The CanalGetStatus call returns the status structure with the channel_status member having the following meaning:

   * Bit 0-15 PEAK Adapter specific (Taken from the CAN_Status method). 
   * Bit 16-28 Reserved.
   * Bit 29 Reserved. 
   * Bit 30 Bus Warning status (repeated from bit 6) .
   * Bit 31 Bus off status (repeat from bit 7).

## example

For the CANUSB adapter use

    CANUSB;125

which says to use the CANUSB adapter at port 0 and 500 kbps.

If you have two adapters use

    CANUSB;125;1

for the second adapter.


[filename](./bottom_copyright.md ':include')
