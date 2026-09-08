# VSCP library reference

This project builds a set of static and shared libraries for the VSCP protocol stack. The library names are generated in the root build tree as `.so` files on Linux, `.dylib` files on macOS, and `.dll` files on Windows.

Typical project setup:

```cmake
cmake_minimum_required(VERSION 3.16)
project(myapp LANGUAGES CXX)

add_subdirectory(external/vscp)

add_executable(myapp main.cpp)
target_link_libraries(myapp PRIVATE vscp_guid vscp_core vscp_client_tcp)
```

A minimal C++ example:

```cpp
#include <vscp/guid.h>
#include <iostream>

int main() {
    cguid id("FF:EE:DD:CC:BB:AA:99:88:77:66:55:44:33:22:11:00");
    std::cout << id.getAsString() << '\n';
    return 0;
}
```

## Library list

| Library | Purpose |
| --- | --- |
| vscp_sockettcp | Low-level socket and TCP support |
| vscp_util | Shared helper functions and crypto utilities |
| vscp_guid | GUID storage, conversion and comparison |
| vscp_guidparser | GUID string parsing helpers |
| vscp_mdf | MDF parser and metadata handling |
| vscp_core | Core protocol and event helpers |
| vscp_client_base | Common client infrastructure |
| vscp_client_canal | CANal client support |
| vscp_client_tcp | TCP client support |
| vscp_client_udp | UDP client support |
| vscp_client_ws1 | WebSocket client support (version 1) |
| vscp_client_ws2 | WebSocket client support (version 2) |
| vscp_client_multicast | Multicast client support |
| vscp_client_mqtt | MQTT client support |
| vscp_client_socketcan | Linux SocketCAN client support |
| vscp_common | Aggregated common stack library |
| vscp_all | Single all-in-one aggregate shared library |

---

## vscp_sockettcp

Purpose: portable TCP socket helper code used by the VSCP stack.

Example:

```cmake
target_link_libraries(app PRIVATE vscp_sockettcp)
```

```cpp
#include <vscp/sockettcp.h>

int main() {
    // Open a TCP connection or use the helper layer in your VSCP client.
    return 0;
}
```

## vscp_util

Purpose: shared utility functions such as base64, CRC, AES, config-file helpers, and password generation.

Example:

```cmake
target_link_libraries(app PRIVATE vscp_util)
```

```cpp
#include <vscp/util/configfile.h>

int main() {
    // Load or write configuration data using VSCP utility helpers.
    return 0;
}
```

## vscp_guid

Purpose: GUID class `cguid` that handles byte-array, string, and compact GUID formats.

Example:

```cmake
target_link_libraries(app PRIVATE vscp_guid)
```

```cpp
#include <vscp/guid.h>
#include <iostream>

int main() {
    cguid guid("01:02:03:04:05:06:07:08:09:0A:0B:0C:0D:0E:0F:10");
    std::cout << guid.toStringUUID() << '\n';
    return 0;
}
```

## vscp_guidparser

Purpose: parsing and formatting GUID strings for C code paths.

Example:

```cmake
target_link_libraries(app PRIVATE vscp_guidparser)
```

```c
#include <vscp/vscp-guid-parser.h>

int main(void) {
    unsigned char guid[16] = {0};
    const char *text = "FF:EE:DD:CC:BB:AA:99:88:77:66:55:44:33:22:11:00";
    vscp_guid_parse(guid, text, NULL);
    return 0;
}
```

## vscp_mdf

Purpose: MDF metadata parsing used by VSCP configuration and device definitions.

Example:

```cmake
target_link_libraries(app PRIVATE vscp_mdf)
```

```cpp
#include <vscp/mdf.h>

int main() {
    // Load and parse an MDF file for device metadata.
    return 0;
}
```

## vscp_core

Purpose: core protocol and event logic used across the stack.

Example:

```cmake
target_link_libraries(app PRIVATE vscp_core)
```

```cpp
#include <vscp/vscphelper.h>

int main() {
    // Use VSCP event helpers, conversions and protocol support.
    return 0;
}
```

## vscp_client_base

Purpose: common client abstraction for transport-independent client logic.

Example:

```cmake
target_link_libraries(app PRIVATE vscp_client_base)
```

```cpp
#include <vscp/vscp-client-base.h>

int main() {
    // Create or configure a client object derived from the base client class.
    return 0;
}
```

## vscp_client_canal

Purpose: client support for CANal-based communication.

Example:

```cmake
target_link_libraries(app PRIVATE vscp_client_canal)
```

```cpp
#include <vscp/vscp-client-canal.h>

int main() {
    // Connect to a Canal-backed VSCP endpoint.
    return 0;
}
```

## vscp_client_tcp

Purpose: TCP client support for VSCP services.

Example:

```cmake
target_link_libraries(app PRIVATE vscp_client_tcp)
```

```cpp
#include <vscp/vscp-client-tcp.h>

int main() {
    // Connect to a VSCP daemon over TCP.
    return 0;
}
```

## vscp_client_udp

Purpose: UDP client support for broadcast and unicast message exchange.

Example:

```cmake
target_link_libraries(app PRIVATE vscp_client_udp)
```

```cpp
#include <vscp/vscp-client-udp.h>

int main() {
    // Use the UDP client to exchange VSCP messages.
    return 0;
}
```

## vscp_client_ws1 and vscp_client_ws2

Purpose: WebSocket client support for VSCP communication over browser-friendly transports.

Example:

```cmake
target_link_libraries(app PRIVATE vscp_client_ws1 vscp_client_ws2)
```

```cpp
#include <vscp/vscp-client-ws1.h>
#include <vscp/vscp-client-ws2.h>

int main() {
    // Use one of the websocket-enabled client libraries depending on the server API.
    return 0;
}
```

## vscp_client_multicast

Purpose: multicast receive/transmit client support.

Example:

```cmake
target_link_libraries(app PRIVATE vscp_client_multicast)
```

```cpp
#include <vscp/vscp-client-multicast.h>

int main() {
    // Subscribe to or publish multicast VSCP messages.
    return 0;
}
```

## vscp_client_mqtt

Purpose: MQTT-based VSCP client support.

Example:

```cmake
target_link_libraries(app PRIVATE vscp_client_mqtt)
```

```cpp
#include <vscp/vscp-client-mqtt.h>

int main() {
    // Connect to an MQTT broker and exchange VSCP events.
    return 0;
}
```

## vscp_client_socketcan

Purpose: Linux SocketCAN transport support.

Example:

```cmake
target_link_libraries(app PRIVATE vscp_client_socketcan)
```

```cpp
#include <vscp/vscp-client-socketcan.h>

int main() {
    // Use VSCP over a SocketCAN interface.
    return 0;
}
```

## vscp_common

Purpose: compatibility aggregate library that links the common stack components together.

Example:

```cmake
target_link_libraries(app PRIVATE vscp_common)
```

```cpp
#include <vscp/vscp.h>

int main() {
    // Link the full common library set in one dependency.
    return 0;
}
```

## vscp_all

Purpose: combined all-in-one dynamic library target that links the full VSCP stack.

Example:

```cmake
target_link_libraries(app PRIVATE vscp_all)
```

```cpp
#include <vscp/vscp.h>

int main() {
    // A single shared library import for the VSCP suite.
    return 0;
}
```

---

## Recommended linking pattern

For most applications, the best practice is to link the smallest set required:

```cmake
target_link_libraries(myapp PRIVATE
    vscp_guid
    vscp_core
    vscp_client_tcp
)
```

Use `vscp_common` when you want the compatibility aggregate, and `vscp_all` when you want a single all-in-one shared library bundle.
