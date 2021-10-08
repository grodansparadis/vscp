# vscp_client_level2 

This is one of the classes in the VSCP client collection. The collection defines programmatic functionality to send and receive VSCP events over different transports.

This class uses the [VSCP Level II driver interface]() as its transport medium. This driver interface is mainly used by the VSCP Daemon to communicate with drivers together with Level I/CANAL drivers. 

```json
{
  "name" : "Name of driver",
  "path-driver": "The path to the driver",
  "path-config": "The path to the driver configuration file",
  "guid": "GUID for driver",
}
```

## Configuration

### name
The name can be set to anything you like. It is used to identify the driver.

### path-driver
The path is the path to the driver. It is used to load the driver.

### path-config
The path is the path to the configuration file for the driver. It is used to configure the functionality of the driver.

### guid
The guid for the driver. Very commonly this GUID should have the two lowest bytes set to _00_ to make room for node id's connected to it. This schema can however be different. A typical GUID is:

```
'FF:FF:FF:FF:FF:FF:FF:F5:0C:00:00:00:00:00:00:00'
```

A driver that connect to a single sensor set the full GUID for it here.

## The Level II driver interface

The Level II driver interface is VERY simple. It is used to send and receive VSCP events and nothing more.

### VSCPOpen

```c
long VSCPOpen(const char *pPathConfig, const char *pguid);
```

Open the driver. 

### VSCPClose

```c
int VSCPClose(long handle);
```

Close the driver.

### VSCPWrite

```c
int VSCPWrite(long handle, const vscpEvent *pEvent, unsigned long timeout);
```

Write (send) one VSCP event to the driver. The call is blocking.

### VSCPRead

```c
VSCPRead(long handle, vscpEvent *pEvent, unsigned long timeout)
```

Get one event from the driver if one is available. The call is blocking.


### VSCPGetVersion

```c
unsigned long VSCPGetVersion(void)
```

Get the version for the driver as a 32 bit unsigned long.