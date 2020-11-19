# LIRC Level I Driver

**Available for:** Windows

**Driver for Windows** vscpl1_lircdrv.dll  (vscpl1_lircdrv.lib)

Device driver for that connects to [LIRC](https://www.lirc.org) (Unix) or [WINLIRC ](https://winlirc.sourceforge.net) (Windows) and receive data from IR remotes and sends IR data out.

LIRC/WINLIRC must be functioning on the computer that use this driver. Follow the information on respective site on how to set them up. 

## Parameter String

   "path to config file;lirchost;lircport"

##### path to config file

The full path to the location for the configuration XML-file.

##### lirchost

Hostname for the host the LIRC daemon is running on. If not given "localhost" is assumed.

##### lircport

The port that the LIRC daemon is listening to. If not given the default-port "8765" i used.

## Flags

Not used. 

## Status return

Not used. Platforms

## Configuration file format

In the configuration file it is possible to set what VSCP (Level I or Level II) events or generic CAN messages that should be sent (none, one or many) for a specific key pressed on the remote.

The format for this file is XML and is defined as follows:

[See source](https://github.com/grodansparadis/vscp_software/tree/master/src/vscp/drivers/level1/lirc)

[filename](./bottom_copyright.md ':include')
