# VSCP Level 2 Client Interface for Node.js

A comprehensive Node.js interface for VSCP (Very Simple Control Protocol) Level 2 drivers and devices. This library provides a JavaScript/TypeScript API that mirrors the functionality of the C++ `vscpClientLevel2` class from the VSCP daemon.

## Overview

This Node.js module implements the VSCP Level 2 driver interface as defined in `level2drvdef.h` and the `vscpClientLevel2` class. It provides a high-level, event-driven API for interacting with VSCP Level 2 devices and drivers.

## Features

- **Complete Level 2 Interface**: Full implementation of VSCP Level 2 driver functionality
- **Event-Driven Architecture**: Built on Node.js EventEmitter for responsive event handling
- **TypeScript Support**: Complete TypeScript definitions included
- **Flexible Filtering**: Advanced event filtering capabilities
- **JSON Serialization**: Full support for event and configuration serialization
- **Statistics Tracking**: Built-in statistics and monitoring
- **Debug Support**: Comprehensive debugging and logging options
- **Promise-Based API**: Modern async/await support for all operations

## Installation

```bash
# Install from the VSCP project
npm install ./src/vscp/nodejs

# Or if published to npm
npm install vscp-level2-client
```

## Quick Start

```javascript
const { VscpLevel2Client, VscpEvent, VSCP_CONSTANTS } = require('vscp-level2-client');

async function example() {
    // Create a new Level 2 client
    const client = new VscpLevel2Client({
        interface: '/dev/vscp0',
        flags: VSCP_CONSTANTS.FLAG_ENABLE_DEBUG,
        responseTimeout: 5000
    });
    
    // Initialize and connect
    await client.connect();
    
    // Create and send an event
    const event = new VscpEvent({
        vscp_class: 10,  // MEASUREMENT
        vscp_type: 6,    // TEMPERATURE  
        data: [0x89, 0x02, 0x00, 0x19]  // 25.0°C
    });
    
    event.setCurrentTime();
    await client.send(event);
    
    // Receive events
    const response = await client.receive(1000);
    if (response.error === VSCP_CONSTANTS.ERROR_SUCCESS) {
        console.log('Received event:', response.event);
    }
    
    // Disconnect
    await client.disconnect();
}
```

## API Reference

### VscpLevel2Client

The main client class that implements the VSCP Level 2 interface.

#### Constructor

```javascript
const client = new VscpLevel2Client(options)
```

**Options:**
- `interface` (string): Interface name or path
- `flags` (number): Configuration flags
- `responseTimeout` (number): Response timeout in milliseconds
- `connectionTimeout` (number): Connection timeout in milliseconds
- `maxQueueSize` (number): Maximum event queue size

#### Core Methods

##### init(interface, flags, timeout)
Initialize the client with interface and configuration.

```javascript
const result = client.init('/dev/vscp0', VSCP_CONSTANTS.FLAG_ENABLE_DEBUG, 5000);
```

##### connect()
Connect to the interface.

```javascript
const result = await client.connect();
```

##### disconnect()
Disconnect from the interface.

```javascript
const result = await client.disconnect();
```

##### send(event, timeout)
Send a VSCP event.

```javascript
const event = new VscpEvent({ vscp_class: 10, vscp_type: 6 });
const result = await client.send(event);
```

##### receive(timeout)
Receive a VSCP event.

```javascript
const response = await client.receive(1000);
if (response.error === VSCP_CONSTANTS.ERROR_SUCCESS) {
    console.log('Event received:', response.event);
}
```

#### Information Methods

##### getVersion()
Get interface version information.

```javascript
const response = await client.getVersion();
console.log(`Version: ${response.version.major}.${response.version.minor}`);
```

##### getInterfaces()
Get list of available interfaces.

```javascript
const response = await client.getInterfaces();
console.log('Interfaces:', response.interfaces);
```

##### getCapabilities()
Get interface capabilities.

```javascript
const response = await client.getCapabilities();
console.log('Capabilities:', response.capabilities.toString(16));
```

#### Filter and Queue Methods

##### setFilter(filter)
Set an event filter.

```javascript
const filter = new VscpEventFilter();
filter.setClassAndType(10, 6);  // Temperature measurements only
client.setFilter(filter);
```

##### getCount()
Get number of events in queue.

```javascript
const response = await client.getCount();
console.log('Events in queue:', response.count);
```

##### clear()
Clear the event queue.

```javascript
client.clear();
```

#### Configuration Methods

##### getConfigAsJson()
Get current configuration as JSON.

```javascript
const config = client.getConfigAsJson();
console.log(config);
```

##### initFromJson(config)
Initialize from JSON configuration.

```javascript
const config = '{"interface": "/dev/vscp1", "flags": 0}';
client.initFromJson(config);
```

#### Events

The client emits the following events:

- `connected`: When successfully connected
- `disconnected`: When disconnected
- `eventSent`: When an event is sent (with event data)
- `eventReceived`: When an event is received (with event data)
- `newEvent`: When a new event is available in queue
- `error`: When an error occurs

```javascript
client.on('eventReceived', (event) => {
    console.log(`Received: Class ${event.vscp_class}, Type ${event.vscp_type}`);
});

client.on('error', (error) => {
    console.error('Client error:', error.message);
});
```

### VscpEvent

Represents a VSCP Level 2 event.

#### Constructor

```javascript
const event = new VscpEvent({
    vscp_class: 10,
    vscp_type: 6,
    GUID: [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15],
    data: [0x89, 0x02, 0x00, 0x19]
});
```

#### Properties

- `crc`: CRC checksum
- `obid`: Object ID
- `year`, `month`, `day`, `hour`, `minute`, `second`: Timestamp
- `timestamp`: Relative timestamp in microseconds
- `head`: Header flags
- `vscp_class`: VSCP class
- `vscp_type`: VSCP type
- `GUID`: 16-byte globally unique identifier
- `sizeData`: Data size
- `data`: Event data array

#### Methods

##### setCurrentTime()
Set the event timestamp to current UTC time.

```javascript
event.setCurrentTime();
```

##### getPriority() / setPriority(priority)
Get or set event priority (0-7, where 0 is highest).

```javascript
event.setPriority(VSCP_CONSTANTS.PRIORITY_HIGH >> 5);
console.log('Priority:', event.getPriority());
```

##### isHardCoded() / setHardCoded(hardCoded)
Get or set hard-coded flag.

```javascript
event.setHardCoded(true);
console.log('Hard coded:', event.isHardCoded());
```

##### toJSON() / fromJSON(json)
Serialize/deserialize event to/from JSON.

```javascript
const json = event.toJSON();
const restored = VscpEvent.fromJSON(json);
```

### VscpEventFilter

Event filter for selecting specific events.

#### Constructor

```javascript
const filter = new VscpEventFilter({
    filter_class: 10,
    mask_class: 0xFFFF,
    filter_type: 6,
    mask_type: 0xFFFF
});
```

#### Methods

##### clear()
Clear all filter settings (allow all events).

```javascript
filter.clear();
```

##### setClassAndType(vscp_class, vscp_type)
Set filter for specific class and type.

```javascript
filter.setClassAndType(10, 6);  // Temperature measurements
```

### VSCP_CONSTANTS

Contains all VSCP constants and error codes.

#### Error Codes

- `ERROR_SUCCESS`: Operation successful
- `ERROR_GENERAL`: General error
- `ERROR_INVALID_HANDLE`: Invalid handle
- `ERROR_INVALID_PARAMETER`: Invalid parameter
- `ERROR_TIMEOUT`: Operation timeout
- `ERROR_NOT_CONNECTED`: Not connected
- `ERROR_WRITE_ERROR`: Write error
- `ERROR_READ_ERROR`: Read error

#### Priorities

- `PRIORITY_0` to `PRIORITY_7`: Priority levels (0 = highest, 7 = lowest)
- `PRIORITY_HIGH`, `PRIORITY_NORMAL`, `PRIORITY_MEDIUM`, `PRIORITY_LOW`: Named priorities

#### Flags

- `FLAG_ENABLE_DEBUG`: Enable debug output
- `HEADER_HARD_CODED`: Hard-coded device flag
- `HEADER_NO_CRC`: Disable CRC calculation

## Examples

### Basic Usage

```javascript
const { VscpLevel2Client, VscpEvent, VSCP_CONSTANTS } = require('vscp-level2-client');

// Run the basic usage example
node examples/basic-usage.js
```

### Event Handling

```javascript
// Run the event handling example
node examples/event-handling.js
```

### Event Filtering

```javascript
// Run the filtering example
node examples/filtering.js
```

### Run Tests

```javascript
// Run the test suite
node examples/test-level2.js
```

## Level 2 Driver Interface Mapping

This Node.js interface directly maps to the C++ Level 2 driver interface defined in `level2drvdef.h`:

| C++ Function | JavaScript Method |
|-------------|------------------|
| `LPFNDLL_VSCPOPEN` | `connect()` |
| `LPFNDLL_VSCPCLOSE` | `disconnect()` |
| `LPFNDLL_VSCPWRITE` | `send()` |
| `LPFNDLL_VSCPREAD` | `receive()` |
| `LPFNDLL_VSCPGETVERSION` | `getVersion()` |

Additional functionality from `vscpClientLevel2`:
- Event filtering via `setFilter()`
- Queue management with `getCount()` and `clear()`
- Configuration management
- Statistics tracking
- Event-driven architecture

## TypeScript Support

Complete TypeScript definitions are provided:

```typescript
import { VscpLevel2Client, VscpEvent, VSCP_CONSTANTS } from 'vscp-level2-client';

const client: VscpLevel2Client = new VscpLevel2Client({
    interface: '/dev/vscp0',
    flags: VSCP_CONSTANTS.FLAG_ENABLE_DEBUG
});
```

## Error Handling

All methods return appropriate error codes as defined in the VSCP specification:

```javascript
const result = await client.send(event);
if (result !== VSCP_CONSTANTS.ERROR_SUCCESS) {
    console.error('Send failed with error:', result);
}
```

Async operations return objects with error codes:

```javascript
const response = await client.receive(1000);
if (response.error === VSCP_CONSTANTS.ERROR_TIMEOUT) {
    console.log('Receive timed out');
} else if (response.error === VSCP_CONSTANTS.ERROR_SUCCESS) {
    console.log('Received event:', response.event);
}
```

## License

MIT License - Copyright (C) 2000-2026 Ake Hedman and contributors, the VSCP project

## Contributing

This is part of the VSCP project. Please see the main VSCP repository for contribution guidelines.

## Links

- [VSCP Project](https://www.vscp.org)
- [VSCP Specification](https://docs.vscp.org)
- [VSCP GitHub](https://github.com/grodansparadis/vscp)
- [Level 2 Driver Documentation](https://docs.vscp.org/vscpd/latest/#/level_ii_drivers)