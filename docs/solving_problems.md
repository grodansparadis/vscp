# Solving problems and troubleshooting

The VSCP daemon has many parts and a lot of things can go wrong. In this section you can find some tips on how to solve some of the more common ones.

The VSCP wiki have a [faq](https://github.com/grodansparadis/vscp/wiki/FAQ) that also may be of help.

## The log file

The vscp daemon writes errors and other  information to the log file. You can set how much information you want in the log file in the configuration file.

| Level | Description |
| ----- | ----------- |
| "trace" | Everything is logged |
| "debug" | Everything except trace is logged |
| "info" | info and above is logged |
| "err" | Errors and above is logged |
| "critical" | Only critical messages are logged |
| "off" | No logging |

You can also define a log that log on the console. This is only useful when the VSCP daemon is not run as a service.

The **debug** entry in the configuration file let you set flags to get different debug information from the VSCP Daemon. 

The log file is by default located at 

```bash
sudo cat /var/log/log/vscp/vscpd.log
```

or with

```bash
sudo tail -f /var/log/log/vscp/vscpd.log
```

but there is also [lnav](http://lnav.org/) and other tools that can be used to view the system logs on your system. For a graphical viewer we can recommend [qjournalctl](https://github.com/pentix/qjournalctl).

```
lnav /var/log/vscp/vscpd.log
```

Do not set debug bits for information you are not interested in. There can be much information anyway without getting info from sources you have no problem or interest in.

### Definition of flags

Flags (bits) of the debug number is described [here](https://github.com/grodansparadis/vscp/blob/master/src/vscp/common/vscp_debug.h)

The right hand number in (1 << n), 'n', is the bit position counter from the right. As a number it will be 2^n. So to get  debug messages for level I drivers receive channel bit eight should be set. That is the same as adding 2^8 = 256 to the debug number.

## Driver log files
VSCP Level II drivers write debug information to a log file. The log file for a driver is typically located at __/var/log/vscp/vscp_drv_<driver_name>.log__.

## Config file

One common problem that will stop the VSCP Daemon from running is to have a misconfigured configuration file. This file must be valid JSON and follow the rules. One way to make sure that your configuration file is valid JSON is to use one of the on-line services for this purpose. Just copy the file and paste the content to a site like [this one](https://jsonlint.com/) or [this one](https://jsonformatter.curiousconcept.com/).

## If you need help

If you need som help you can ask your questions and discuss things [here](https://github.com/grodansparadis/vscp/discussions). The VSCP repository is also the place to use for bug reports and similar. [Just open up a new issue for it](https://github.com/grodansparadis/vscp/issues). 

For drivers please use the relevant repository.

## Troubleshooting

The first step in any trouble shooting is of course to log as much as possible. Often you find the problem in the configuration file. 

For problems with publishing on MQTT topics it can be wise to setup additional simple topics on a local server so you can see what is going on.

[filename](./bottom_copyright.md ':include')