# Socketcan Level I Driver

**Available for:** Linux

SocketCAN, the official CAN API of the Linux kernel, has been included in the kernel for many years now. Meanwhile, the official Linux repository has device drivers for all major CAN chipsets used in various architectures and bus types. SocketCAN offers the user a multiuser capable as well as hardware independent socket-based API for CAN based communication and configuration. Socketcan nowadays give access to the major CAN adapters that is available on the market. Note that as CAN only can handle Level I events only events up to class < 1024 can be sent to this device. Other events will be filtered out. Also received events 

**Driver Linux**: vscpl1_socketcandrv.so

## Parameter String

    interface;mask;filter

##### interface

Typically **"any"** or something like **"vcan0", "can0", "can1", "can2"** 

##### mask

is the mask for the adapter.  

##### filter

is the filter for the adapter. 

## Flags

Not used, set to 0. 
 
## Notes

*  Some notes about socketcan is [here](https://www.akehedman.se/wiki/doku.php/socketcan). 

[filename](./bottom_copyright.md ':include')
