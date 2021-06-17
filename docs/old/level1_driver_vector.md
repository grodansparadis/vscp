# Vector CAN Level I Driver

**Available for:** Windows

**Driver for Windows** vscpl1_vectordrv.dll (vscpl1_vectordrv.lib)

This is a device driver for the [Vector Informatik](https://www.vector-informatik.com/english). The driver uses proprietary include files and libraries from Vector which are not included in the distribution.  

The driver works for the following hardware

    CANcardXL
    CANcardXLe
    CANcaseXL
    CANboardXL
    CANboardXL pxi
    CANboardXL PCIe 
    VN2600/VN2610/VN2640
    VN3300/VN3600/VN7600
    VN8910/VN8910A
    VN16xx
    VN7570
    VN5610

## Parameter String

The driver string has the following format (note that all values can be entered in either decimal or hexadecimal form (for hex precede with 0x).

    board;boardidx;channel;filter;mask;bus-speed

##### board

board is one of

   * VIRTUAL or 1 for virtual adapter 
   * CANCARDX or 2 
   * CANPARI or 3 
   * CANAC2 or 5 
   * CANAC2PCI or 6 
   * CANCARDY or 12 
   * CANCARDXL or 15 for PCI Card / PCMCIA 
   * CANCARD2 or 17 
   * EDICCARD or 19 
   * CANCASEXL or 21 for USB adapter 
   * CANBOARDXL or 25 
   * CANBOARDXL_COMPACT or 27 

##### board index

index for board if more then one of same type (0…n)

##### channel

Is a value from 0 an up indicating the CAN channel on the selected board.

##### filter

Is the hardware dependent filter for this board hardware. Note that this

filter may work in a different way then the CANAL filter.

##### mask

Is the hardware dependent mask for this board hardware. Note that this filter may work in a different way then the CANAL filter. 

##### bus_speed

The actual bit rate is set here i.e. 125b is given as 125,000 

## Flags

   * bit 0 Reserved
   * bit 1 Reserved

##### bit 2

   * 0 Filter our own TX messages from our receive.. 
   * 1 All sent CAN objects appear as received CAN objects in the rx queues.

##### bit 3

   * 0 Active Mode. 
   * 1 Passive mode: CAN controller works as passive CAN node (only monitoring) and therefore it does not send any acknowledge bit for CAN objects sent by another CAN node.

##### bit 4

   * 0 No error report objects. 
   * 1 Error frames are detected and reported as CAN objects via the rx queues. 

## Status return

The CanalGetStatus call returns the status structure with the channel_status member having the following meaning:

   * Bit 0-7 TX error counter 
   * Bit 8-15 RX error counter 
   * Bit 16-17 Reserved 
   * Bit 18 All sent CAN objects appear as received CAN objects. 
   * Bit 19 Passive mode. 
   * Bit 20-27 Reserved. 
   * Bit 28 Active
   * Bit 29 Bus Passive. 
   * Bit 30 Bus Warning status. 
   * Bit 31 Bus off status. 

[filename](./bottom_copyright.md ':include')
