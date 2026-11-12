# vscp_client_level2

This is one of the classes in the VSCP client collection. The collection defines programmatic functionality to send and receive VSCP events over different transports.

This class uses the VSCP Level II driver interface as its transport medium. The interface is mainly used by the VSCP daemon, but with this class any Level II driver (`vscpl2drv-*`) can be loaded and used directly by an application such as VSCP Works.

```json
{
  "name" : "Name of connection",
  "path-driver": "The path to the driver",
  "path-config": "The path to the driver configuration file",
  "config": { "embedded": "driver configuration" },
  "config-suffix": ".json",
  "guid": "GUID for driver",
  "response-timeout": 1000
}
```

## Configuration

### name
The name can be set to anything you like. It is used to identify the connection.

### path-driver
The path to the driver shared library.

### path-config
The path to the configuration file for the driver.

### config
The driver configuration embedded in the connection configuration. It can be a JSON object or array (written as JSON) or a string (written as is, for example XML). When the client connects the configuration is written to a new temporary file that only the owner can read, and the path to that file is given to the driver. The file is removed when the client disconnects. If `config` is set it is used instead of `path-config`.

### config-suffix
Suffix for the temporary configuration file, for example `.json` or `.xml`. If not set it is guessed from the content (`.json` for `{` or `[`, `.xml` for `<`, `.conf` otherwise).

### guid
The guid for the driver. Very commonly this GUID should have the two lowest bytes set to _00_ to make room for node id's connected to it. This schema can however be different. A typical GUID is:

```
'FF:FF:FF:FF:FF:FF:FF:F5:0C:00:00:00:00:00:00:00'
```

A driver that connect to a single sensor set the full GUID for it here.

### response-timeout
Timeout in milliseconds for writing an event to the driver.

### Filter
`priority-filter`, `priority-mask`, `class-filter`, `class-mask`, `type-filter`, `type-mask`, `guid-filter` and `guid-mask` set the receive filter as for the other clients.

## The Level II driver interface

The Level II driver interface is VERY simple. It is used to send and receive VSCP events and nothing more.

### VSCPOpen

```c
long VSCPOpen(const char *pPathConfig, const unsigned char *pguid);
```

Open the driver with the path to its configuration file and a pointer to the 16 byte GUID. Returns a handle, or 0 on failure.

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