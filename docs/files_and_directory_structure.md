# Files and directory structure

The folders VSCP files are installed to has changed extensively from version 14.0. The reason for moving files to new install folders is to adopt to [lsb](https://refspecs.linuxfoundation.org/lsb.shtml) and other standards for Linux system that mandate where files should be installed.

## Folder content on Linux

| Folder      | Content | 
| ------      | ------- | 
| /usr/sbin   | The VSCP daemon executable is installed here. | 
| _/var/lib/vscp/drivers/level1_ and _/var/lib/vscp/drivers/level1_ | Level I/II drivers names as _vscpl1drv-xxxxx, vscpl2drv-xxxxx. Level I drivers are installed in the level1 subfolder and level II drivers in the level2 sub folder_ often in there own subfolder. |
| /etc/vscp | Configuration files are here. And especially vscpd.json, thew VSCP daemon configuration file. |
| /etc/vscp/ca_certificates  | Place your SSL/TLS Certificate Authority certificates in this directory. | 
| /etc/vscp/certs  | Place your SSL/TLS server keys and certificates in this directory. **This directory should only be readable by the vscp user.** |
| /var/lib/vscp | Persistent storage for vscpd and other software and tools related to VSCP. This is the default location for databases also. |
| /var/lib/vscp/web | The VSCP daemon package creates the web folder structure even it does not use them. This is mainly intended for the [vscpl2drv_webserv driver](https://github.com/grodansparadis/vscpl2drv-websrv) |
| /run | lock file vscpd.lock |
| /usr/share/vscp | Shared VSCP date such as config examples etc. | 

    
[filename](./bottom_copyright.md ':include')

