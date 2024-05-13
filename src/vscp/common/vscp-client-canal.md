# vscp_client_canal 

This is one of the classes in the VSCP client collection. The collection defines programmatic functionality to send and receive VSCP events over different transports.

This class uses the [CANAL protocol](https://docs.vscp.org/#canal) as its transport medium. CANAL stands for CAN Abstraction Layer and is a way to abstract the physical CAN bus. It is used as the lowest common denominator for VSCP events.

```json
{
  "name" : "Name of driver",
  "path" : "Path to driver",
  "config" : "CANAL configuration string",
  "flags" : 0
}
```

## Configuration

### name
The name can be set to anything you like. It is used to identify the driver.

### path
The path is the path to the driver. It is used to load the driver.

### config
The configuration string is used to configure the driver. Each CANAL driver have it's own configuration string described in the driver documentation.

### flags
The flags value are used to set driver specific options. It's a 32-bit value. Bit 31 (zero offset) is often used as a flag to indicate that the driver should output debug information.

