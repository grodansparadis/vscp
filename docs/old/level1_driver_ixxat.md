# IXATT VCI Level I Driver

**Available for:** Windows

**Driver for Windows** vscpl1_ixxatvcidrv.dll (vscpl1_ixxatvcidrv.lib)

This is a device driver for the IXXAT VCI Driver interface

The driver uses proprietary include files and libraries from IXXAT which are not included in the distribution. [IXXAT - Overview PC/CAN Interfaces](https://www.ixxat.com/download_vci_v3_en.html).


## Parameter String

The driver string has the following format (note that all values can be entered in either decimal or hexadecimal form (for hex precede with 0x).

    board;channel;filter;mask;bus-speed;btr0;btr1

##### board

board should be one of: 

   * VCI_IPCI165 or 0 for iPC-I 165, ISA slot 
   * VCI_IPCI320 or 1 for iPC-I 320, ISA slot 
   * VCI_CANDY or 2 for CANdy320, LPT port 
   * VCI_PCMCIA or 3 for tinCAN, pc card 
   * VCI_IPCI386 or 5 for iPC-I 386, ISA slot 
   * VCI_IPCI165_PCI or 6 for iPC-I 165, PCI slot 
   * VCI_IPCI320_PCI or 7 for iPC-I 320, PCI slot 
   * VCI_CP350_PCI or 8 for iPC-I 165, PCI slot 
   * VCI_PMC250_PCI or 9 for special hardware from PEP 
   * VCI_USB2CAN or 10 for USB2CAN, USB port 
   * VCI_CANDYLITE or 11 for CANdy lite, LPT port 
   * VCI_CANANET or 12 for CAN@net, Ethernet 
   * VCI_BFCARD or 13 for byteflight Card, pc card 
   * VCI_PCI04_PCI or 14 for PC-I 04, PCI slot, passive 
   * VCI_USB2CAN_COMPACT or 15 for USB2CAN compact, USB port 
   * VCI_PASSIV or 50 for PC-I 03, ISA slot, passive

##### channel

channel Is a value from 0 an up indicating the CAN channel on the selected board.

** filter **
filter Is the hardware dependent filter for this board hardware. Note that this filter may work in a different way then the CANAL filter.

##### mask

mask Is the hardware dependent mask for this board hardware. Note that this filter may work in a different way then the CANAL filter.

##### bus-speed

One of the predefined bit rates can be set here 10 for 10 Kbps 20 for 20 Kbps 50 for 50 Kbps 100 for 100 Kbps 125 for 125 Kbps 250 for 250 Kbps 500 for 500 Kbps 800 for 800 Kbps 1000 for 1000 Mobs

##### btr0/btr1

Value for bit rate register 0/1. If btr value pairs are given then the bus-speed parameter is ignored.

## Flags

##### bit 0

   * 0 11-bit identifier mode 
   * 1 29-bit identifier mode

##### bit 1

   * 0 High speed 
   * 1 A low speed-bus connector is used (if provided by the hardware)

##### bit 2

   * 0 Filter our own TX messages from our receive.. 
   * 1 All sent CAN objects appear as received CAN objects in the rx queues.

##### bit 3

   * 0 Active Mode. 
   * 1 Passive mode: CAN controller works as passive CAN node (only monitoring) therefore it does not send any acknowledge bit for CAN objects sent by another CAN node.

##### bit 4


*  0 No error report objects. 

*  1 Error frames are detected and reported as CAN objects via the rx queues

## Status return

The CanalGetStatus call returns the status structure with the channel_status member having the following meaning:

   * Bit 0-1 Reserved. 
   * Bit 2 RemoteQueOverrun 
   * Bit 3 CAN-TX pending 
   * Bit 4 CAN-Init-Mode. 
   * Bit 5 CAN-Data-Overrun. 
   * Bit 6 CAN-Error-Warning-Level. 
   * Bit 7 CAN_Bus_Off_status. 
   * Bit 8-15 Reserved. 
   * Bit 16 29-bit ID. Bit 17 Low speed-bus connector. 
   * Bit 18 All sent CAN objects appear as received CAN objects. 
   * Bit 19 Passive mode Bit 20 Error frames are detected and reported. 
   * Bit 21-29 Reserved. Bit 30 Bus Warning status (repeated from bit 6) 
   * Bit 31 Bus off status (repeated from bit 7)

##  Examples 

    VCI_USB2CAN_COMPACT;0;0;0;500

uses the USB2CAN compact USB adapter with a bus-speed off 500kbps and the first (and only) channel of the adapter with filter/mask fully open. This can also be set as

    15;0;0;0;125

[filename](./bottom_copyright.md ':include')
