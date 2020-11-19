# Logging

System messages like errors, warnings and notifications from the VSCP daemon is logged to syslog. This means you can find this information in the syslog file

    /var/log/syslog

You can use a log viewer like **lnav** to view output ([others here](https://www.ubuntupit.com/best-linux-log-viewer-and-log-file-management-tools/)) or you can use 

```bash
tail -f /var/log/syslog
```

or

```bash
tail -f /var/log/syslog | grep vscpd
```

or even

```bash
sudo journalctl -f -u vscp -o cat
```

## Level of information

Only errors and severe warnings and some startup information is normally logged. With the debug flag settings in the configuration file you can output a lot more information that can be useful when diagnosing a problem.

The debug flags consist of eight bytes. Each bit is a flag that enable some specific debug output.

The flags that can be set and there meaning is described [here](https://docs.vscp.org/vscpd/latest/#/configuring_the_vscp_daemon?id=config-debugflags)



[filename](./bottom_copyright.md ':include')