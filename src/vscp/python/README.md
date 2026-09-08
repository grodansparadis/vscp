# VSCP Level 2 Client Interface for Python

A comprehensive Python interface for VSCP (Very Simple Control Protocol) Level 2 drivers and devices. This library provides a Python API that mirrors the functionality of the C++ `vscpClientLevel2` class from the VSCP daemon.

## Overview

This Python package implements the VSCP Level 2 driver interface as defined in `level2drvdef.h` and the `vscpClientLevel2` class. It provides a high-level, async/await-based API for interacting with VSCP Level 2 devices and drivers.

## Features

- **Complete Level 2 Interface**: Full implementation of VSCP Level 2 driver functionality
- **Async/Await Support**: Modern Python asynchronous programming with asyncio
- **Type Hints**: Complete type annotations for better IDE support and code safety
- **Dataclass-based Events**: Clean, immutable-like event structures using Python dataclasses
- **Flexible Filtering**: Advanced event filtering capabilities
- **JSON Serialization**: Full support for event and configuration serialization
- **Statistics Tracking**: Built-in statistics and monitoring
- **Debug Support**: Comprehensive logging integration with Python's logging module
- **Thread-Safe**: Proper threading support for multi-threaded applications

## Installation

```bash
# Install from the VSCP project
pip install ./src/vscp/python

# Or install in development mode
pip install -e ./src/vscp/python[dev]
```

## Quick Start

```python
import asyncio
from vscp_level2 import VSCPLevel2Client, VSCPEvent, VSCPError, VSCPConstants

async def example():
    # Create a new Level 2 client
    client = VSCPLevel2Client(
        interface='/dev/vscp0',
        flags=VSCPConstants.FLAG_ENABLE_DEBUG,
        response_timeout=5000
    )
    
    # Initialize and connect
    await client.connect()
    
    # Create and send an event
    event = VSCPEvent(
        vscp_class=10,  # MEASUREMENT
        vscp_type=6,    # TEMPERATURE  
        data=[0x89, 0x02, 0x00, 0x19]  # 25.0°C
    )
    
    event.set_current_time()
    await client.send(event)
    
    # Receive events
    error_code, received_event = await client.receive(1000)
    if error_code == VSCPError.SUCCESS:
        print(f'Received event: Class {received_event.vscp_class}, Type {received_event.vscp_type}')
    
    # Disconnect
    await client.disconnect()

# Run the example
asyncio.run(example())
```

## API Reference

### VSCPLevel2Client

The main client class that implements the VSCP Level 2 interface.

#### Constructor

```python
client = VSCPLevel2Client(
    interface='/dev/vscp0',
    flags=VSCPConstants.FLAG_ENABLE_DEBUG,
    response_timeout=5000,
    connection_timeout=5000,
    max_queue_size=1000
)
```

**Parameters:**
- `interface` (str): Interface name or path
- `flags` (int): Configuration flags
- `response_timeout` (int): Response timeout in milliseconds
- `connection_timeout` (int): Connection timeout in milliseconds
- `max_queue_size` (int): Maximum event queue size

#### Core Methods

##### init(interface, flags, timeout)
Initialize the client with interface and configuration.

```python
result = client.init('/dev/vscp0', VSCPConstants.FLAG_ENABLE_DEBUG, 5000)
```

##### async connect()
Connect to the interface.

```python
result = await client.connect()
```

##### async disconnect()
Disconnect from the interface.

```python
result = await client.disconnect()
```

##### async send(event, timeout=None)
Send a VSCP event.

```python
event = VSCPEvent(vscp_class=10, vscp_type=6)
result = await client.send(event)
```

##### async receive(timeout=None)
Receive a VSCP event.

```python
error_code, event = await client.receive(1000)
if error_code == VSCPError.SUCCESS:
    print(f'Received: {event}')
```

#### Information Methods

##### async get_version()
Get interface version information.

```python
error_code, version_info = await client.get_version()
print(f'Version: {version_info.major}.{version_info.minor}.{version_info.release}')
```

##### async get_interfaces()
Get list of available interfaces.

```python
error_code, interfaces = await client.get_interfaces()
print(f'Interfaces: {interfaces}')
```

##### async get_capabilities()
Get interface capabilities.

```python
error_code, capabilities = await client.get_capabilities()
print(f'Capabilities: 0x{capabilities:x}')
```

#### Filter and Queue Methods

##### set_filter(filter_obj)
Set an event filter.

```python
filter_obj = VSCPEventFilter()
filter_obj.set_class_and_type(10, 6)  # Temperature measurements only
client.set_filter(filter_obj)
```

##### async get_count()
Get number of events in queue.

```python
error_code, count = await client.get_count()
print(f'Events in queue: {count}')
```

##### clear()
Clear the event queue.

```python
client.clear()
```

#### Configuration Methods

##### get_config_as_json()
Get current configuration as JSON.

```python
config = client.get_config_as_json()
print(config)
```

##### init_from_json(config)
Initialize from JSON configuration.

```python
config = '{"interface": "/dev/vscp1", "flags": 0}'
client.init_from_json(config)
```

### VSCPEvent

Represents a VSCP Level 2 event using Python dataclasses.

#### Constructor

```python
from vscp_level2 import VSCPEvent

event = VSCPEvent(
    vscp_class=10,
    vscp_type=6,
    guid=[0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15],
    data=[0x89, 0x02, 0x00, 0x19]
)
```

#### Properties

- `crc`: CRC checksum
- `obid`: Object ID
- `year`, `month`, `day`, `hour`, `minute`, `second`: Timestamp components
- `timestamp`: Relative timestamp in microseconds
- `head`: Header flags
- `vscp_class`: VSCP class
- `vscp_type`: VSCP type
- `guid`: 16-byte globally unique identifier (List[int])
- `size_data`: Data size (automatically calculated)
- `data`: Event data array (List[int])

#### Methods

##### set_current_time()
Set the event timestamp to current UTC time.

```python
event.set_current_time()
```

##### get_priority() / set_priority(priority)
Get or set event priority (0-7, where 0 is highest).

```python
from vscp_level2 import VSCPPriority

event.set_priority(VSCPPriority.HIGH >> 5)
priority = event.get_priority()
```

##### is_hard_coded() / set_hard_coded(hard_coded)
Get or set hard-coded flag.

```python
event.set_hard_coded(True)
is_hard_coded = event.is_hard_coded()
```

##### to_dict() / from_dict(data)
Serialize/deserialize event to/from dictionary.

```python
event_dict = event.to_dict()
restored_event = VSCPEvent.from_dict(event_dict)
```

##### to_json() / from_json(json_str)
Serialize/deserialize event to/from JSON string.

```python
event_json = event.to_json()
restored_event = VSCPEvent.from_json(event_json)
```

### VSCPEventFilter

Event filter for selecting specific events.

#### Constructor

```python
from vscp_level2 import VSCPEventFilter

filter_obj = VSCPEventFilter(
    filter_class=10,
    mask_class=0xFFFF,
    filter_type=6,
    mask_type=0xFFFF
)
```

#### Methods

##### clear()
Clear all filter settings (allow all events).

```python
filter_obj.clear()
```

##### set_class_and_type(vscp_class, vscp_type)
Set filter for specific class and type.

```python
filter_obj.set_class_and_type(10, 6)  # Temperature measurements
```

### Constants and Enums

#### VSCPError
Error codes returned by operations.

```python
from vscp_level2 import VSCPError

if result == VSCPError.SUCCESS:
    print("Operation successful")
elif result == VSCPError.TIMEOUT:
    print("Operation timed out")
elif result == VSCPError.NOT_CONNECTED:
    print("Not connected to interface")
```

#### VSCPPriority
Event priority levels.

```python
from vscp_level2 import VSCPPriority

event.set_priority(VSCPPriority.HIGH >> 5)      # Highest priority
event.set_priority(VSCPPriority.NORMAL >> 5)    # Normal priority
event.set_priority(VSCPPriority.LOW >> 5)       # Lowest priority
```

#### VSCPConstants
Various VSCP constants.

```python
from vscp_level2 import VSCPConstants

# Ports
print(f"Default TCP port: {VSCPConstants.DEFAULT_TCP_PORT}")

# Limits
print(f"Max data size: {VSCPConstants.MAX_DATA}")
print(f"GUID size: {VSCPConstants.SIZE_GUID}")

# Flags
client = VSCPLevel2Client(flags=VSCPConstants.FLAG_ENABLE_DEBUG)
```

## Examples

### Basic Usage

```python
# Run the basic usage example
python examples/basic_usage.py
```

### Event Handling

```python
# Run the event handling example
python examples/event_handling.py
```

### Event Filtering

```python
# Run the filtering example
python examples/filtering.py
```

### Run Tests

```python
# Run with pytest (if available)
pytest tests/test_level2_client.py -v

# Or run simple tests
python tests/test_level2_client.py
```

## Level 2 Driver Interface Mapping

This Python interface directly maps to the C++ Level 2 driver interface defined in `level2drvdef.h`:

| C++ Function | Python Method |
|-------------|---------------|
| `LPFNDLL_VSCPOPEN` | `connect()` |
| `LPFNDLL_VSCPCLOSE` | `disconnect()` |
| `LPFNDLL_VSCPWRITE` | `send()` |
| `LPFNDLL_VSCPREAD` | `receive()` |
| `LPFNDLL_VSCPGETVERSION` | `get_version()` |

Additional functionality from `vscpClientLevel2`:
- Event filtering via `set_filter()`
- Queue management with `get_count()` and `clear()`
- Configuration management with JSON serialization
- Statistics tracking
- Async/await support for modern Python

## Type Hints and IDE Support

The library includes comprehensive type hints for better IDE support:

```python
from typing import Tuple, Optional, List
from vscp_level2 import VSCPLevel2Client, VSCPEvent, VSCPError

async def handle_events(client: VSCPLevel2Client) -> None:
    error_code: int
    event: Optional[VSCPEvent]
    
    error_code, event = await client.receive(1000)
    
    if error_code == VSCPError.SUCCESS and event is not None:
        data: List[int] = event.data
        # IDE will provide proper autocomplete and type checking
```

## Logging Integration

The library integrates with Python's standard logging module:

```python
import logging
from vscp_level2 import VSCPLevel2Client, VSCPConstants

# Enable debug logging
logging.basicConfig(level=logging.DEBUG)

# Create client with debug flag
client = VSCPLevel2Client(
    interface='/dev/vscp0',
    flags=VSCPConstants.FLAG_ENABLE_DEBUG
)
```

## Threading Support

The client is thread-safe and can be used in multi-threaded applications:

```python
import threading
import asyncio
from vscp_level2 import VSCPLevel2Client

async def event_sender(client):
    while True:
        event = VSCPEvent(vscp_class=10, vscp_type=6)
        await client.send(event)
        await asyncio.sleep(1)

async def event_receiver(client):
    while True:
        error_code, event = await client.receive(1000)
        if error_code == VSCPError.SUCCESS:
            print(f'Received: {event}')

async def main():
    client = VSCPLevel2Client()
    await client.connect()
    
    # Run sender and receiver concurrently
    await asyncio.gather(
        event_sender(client),
        event_receiver(client)
    )
```

## Error Handling

All methods return appropriate error codes as defined in the VSCP specification:

```python
from vscp_level2 import VSCPError

result = await client.send(event)
if result != VSCPError.SUCCESS:
    print(f'Send failed with error: {result}')

# Async operations return tuples with error codes
error_code, event = await client.receive(1000)
if error_code == VSCPError.TIMEOUT:
    print('Receive timed out')
elif error_code == VSCPError.SUCCESS:
    print(f'Received event: {event}')
```

## Development

### Setting up development environment

```bash
# Clone the repository
git clone https://github.com/grodansparadis/vscp.git
cd vscp/src/vscp/python

# Install in development mode with dev dependencies
pip install -e .[dev]

# Run tests
pytest tests/ -v

# Run type checking
mypy vscp_level2/

# Format code
black vscp_level2/ examples/ tests/
isort vscp_level2/ examples/ tests/

# Lint code
flake8 vscp_level2/ examples/ tests/
```

### Building and Distribution

```bash
# Build package
python -m build

# Install locally
pip install dist/vscp_level2_client-*.whl
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
- [Python asyncio Documentation](https://docs.python.org/3/library/asyncio.html)