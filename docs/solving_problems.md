# Solving problems and troubleshooting

The VSCP daemon has many parts and a lot of things can go wrong. In this section you can find some tips on how to solve some of the more common ones.

## Debug flags

The **debug** entry in the configuration file let you set flags to get different debug information from the VSCP Daemon. This information is written to the syslog file, a log file that is located at /var/log/syslog. You can view this file with

```bash
sudo cat /var/log/syslog
```

or with

```bash
sudo tail -f /var/log/syslog
```

but there is also [lnav](http://lnav.org/) and other tools that can be used to view the system logs on your system. For a graphical viewer we can recommend [qjournalctl](https://github.com/pentix/qjournalctl).

Do not set debug bits for information you are not interested in. There can be much information anyway without getting info from sources you have no problem or interest in.

### Definition of flags

Flags (bits) of the debug number is described [here](https://github.com/grodansparadis/vscp/blob/master/src/vscp/common/vscp_debug.h)

The right hand number in (1 << n), 'n', is the bit position counter from the right. As a number it will be 2^n. So to get  debug messages for level I drivers receive channel bit eight should be set. That is the same as adding 2^8 = 256 to the debug number.

## Config file

One common problem that will stop the VSCP Daemon from running is to have a misconfigured configuration file. This file must be valid JSON and follow the rules. One way to make sure that your configuration file is valid JSON is to use one of the on-line services for this purpose. Just copy the file and paste the content to a site like [this one](https://jsonlint.com/) or [this one](https://jsonformatter.curiousconcept.com/).

## If you need help

If you need som help you can ask your questions and discuss things [here](https://github.com/grodansparadis/vscp/discussions). The VSCP repository is also the place to use for bug reports and similar. [Just open up a new issue for it](https://github.com/grodansparadis/vscp/issues). 

For drivers please use the relevant repository.

[filename](./bottom_copyright.md ':include')