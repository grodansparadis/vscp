# Logger Level I Driver

**Available for:** Windows, Linux

**Driver for Windows** vscpl1_loggerdrv.dll (vscpl1_loggerdrv.lib)

Device driver for diagnostic logging. It allows you to log CAN (VSCP Level I) traffic to a text file. Several drivers can be loaded with different output files and using different filter/masks. 

## Parameter String

##### Windows

    c:\logfile.log;0x0;0x0

##### Linux

    /tmp/logfile;0x0;0x0

The absolute or relative path including the file name to the file that log data should be written to. The filename is followed by the optional 32-bit filter and mask. The default is all events logged.

Note that the filter/mask looks at the CAN ID. If you work with VSCP look at format of the 29 bit CAN identifier in VSCP Bit 32 is set to one for an extended frame (all VSCP frames) and bit 30 is set to one for RTR frames (never for VSCP). 

## Flags

   * 0 – Append data to an existing file (Create if not available). 
   * 1 – Create a new file or rewrite data on an old file.

## Status return

The CanalGetStatus call returns the status structure with the channel_status member having the following meaning: 


*  0 is always returned. 

## Log file format

The log file have the following format and consist of the following parts

   * Time when frame was received 
   * Timestamp 
   * Flags 
   * ID 
   * Number of databytes 
   * Databytes 


[filename](./bottom_copyright.md ':include')
