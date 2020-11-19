# Files and directory structure

The folders VSCP files are installed to has changed extensively from version 14.0. The reason for moving files to new install folders is to adopt to [lsb](https://refspecs.linuxfoundation.org/lsb.shtml) and other standards for Linux system that mandate where files should be installed.

## Folder content on Linux

| Folder      | Content | 
| ------      | ------- | 
| /usr/sbin   | The VSCP daemon is installed here. | 
| /usr/lib | Level I/II drivers names as _vscpl1drv-xxxxx, vscpl2drv-xxxxx_ |
| /etc/vscp | Configuration files are here. Ans especially vscpd.conf, thew VSCP daemon configuration file. |
| /etc/ca_certificates  | Place your SSL/TLS Certificate Authority certificates in this directory. | 
| /etc/certs  | Place your SSL/TLS server keys and certificates in this directory. **This directory should only be readable by the vscp user.** |
| /var/lib/vscp | Persistent storage for vscpd and other software and tools related to VSCP. |
| /var/log | web server logging if enabled. | 
| /run | lock file vscpd.lock |
| /usr/share/vscp | Shared VSCP date such as config examples etc. | 




    
[filename](./bottom_copyright.md ':include')

