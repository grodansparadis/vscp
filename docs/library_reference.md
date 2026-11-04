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

## Helper and TCP logging

`vscp_util` contains the private, synchronous logger used by `vscphelper.cpp`,
inline functions in `vscphelper.h`, and `VscpRemoteTcpIf` in `vscp_core`.
Both libraries export the configuration API and share the same logging state.
Before configuration, errors go to stderr and debug tracing is disabled.
Applications can configure independent stderr and rotating-file thresholds
through the C-compatible `vscphelperlogging.h` interface without including or
linking spdlog themselves:

```c
#include <vscp.h>
#include <vscphelperlogging.h>

int result = vscphelper_configureLogging(
    VSCP_HELPER_LOG_LEVEL_WARN, VSCP_HELPER_LOG_LEVEL_DEBUG,
    "/path/owned/by/application/vscp.log", 5u * 1024u * 1024u, 3);
if (result != VSCP_ERROR_SUCCESS) {
    /* vscphelper_getLastLoggingError() describes the failure. */
}
```

Use `VSCP_HELPER_LOG_LEVEL_OFF` to disable either sink, and
`vscphelper_flushLogging()` or `vscphelper_disableLogging()` to flush or disable
both sinks. Logging and configuration are thread-safe. File rotation keeps the
requested number of archives in addition to the active file; a single message
may exceed the rotation size. File logging requires a nonempty path, positive
size, and 1 through 200000 archives. Failures are reported to stderr and through
the thread-local `vscphelper_getLastLoggingError()` result. Do not unload the
library while it is in use, or share a rotating file between processes.

Debug logging traces function entry and routine receive timeouts; failures are
logged at error level. Normal false predicate results and empty or timed-out
receive polls are not logged as errors. In particular, the TCP `RETR` response
`-OK - No event(s) available` is logged at debug level without changing the
legacy receive return codes. Function traces do not include arguments,
credentials, responses, or event payloads. Some transport errors do include
hostnames, TLS file paths, and operating-system error codes; protect log files
accordingly.

The logger does not alter spdlog's global default or registry and passes no
spdlog types through its C interface. Libraries linked to the same `vscp_util`
share its configuration; a separately built copy has its own state. Standalone
consumers of the inline helpers must link `vscp_util` or define
`VSCP_DISABLE_HELPER_LOGGING` consistently in their translation units. With
that opt-out, helper debug tracing is disabled and helper errors are reported
to stderr. Direct builds of the TCP source must also define
`VSCP_ENABLE_HELPER_LOGGING` and link the logger to enable its debug tracing
and route its diagnostics through the shared logger.
The optional `test_core_logging_host_compat` test can be enabled by setting
`VSCP_TEST_HOST_SPDLOG_INCLUDE_DIR` to headers for a different spdlog version.
See [Logging design](logging-design.md) for why VSCP uses this approach.

## TCP client transport and TLS

`VscpRemoteTcpIf` (and `vscpClientTcp`, which uses it) communicates through
native POSIX sockets or Windows Winsock. TLS uses the existing OpenSSL dependency.

The synchronous command API is unchanged. `tcp://host:port` and `host:port`
select plain TCP; `stcp://host:port` selects TLS. `enableTLS(true)` forces TLS,
`enableTLS(false)` forces plain TCP, and `setTLSAutoSelect()` restores selection
by prefix. Hostname resolution and TLS handshake time are included in the
connection timeout. Except for the single-channel mode described below, use a
client from only one thread at a time. Connection objects are non-copyable.

When a hostname resolves to multiple addresses, connection attempts are
staggered so that a stalled address does not block the others.

If a system DNS lookup outlasts the timeout, it can finish in the background;
unloading the library waits for outstanding lookups to finish.

The former public `VscpRemoteTcpIf::m_conn` Mongoose pointer has been removed.
Consumers using that member must migrate to the client API, and applications
using the C++ library should be rebuilt for the changed class layout.
The `vscp_mongoose` CMake target is no longer provided.
The unbuilt legacy multicast server's `receiveFrame` and `sendFrame` stubs were
also removed.

```cpp
VscpRemoteTcpIf client;
client.setTLSOptions(true, "/etc/vscp/ca.pem");
int result = client.doCmdOpen("stcp://daemon.example.org:9598", "admin", "secret");
```

Certificate and hostname verification are **on by default** for TLS, including
`stcp://` endpoints. Use `setTLSOptions(true, ...)` to configure a private CA
file and/or directory. With neither CA option set,
OpenSSL's default trust paths (including `SSL_CERT_FILE`/`SSL_CERT_DIR`) are
used. Certificate directories contain PEM certificates. Client certificate
and private-key paths enable mutual TLS; encrypted PEM keys use the supplied
key password. Invalid TLS configuration and failed verification return a
connection error; they never fall back to plain TCP.

This intentionally changes the previous unverified TLS default. Deployments
using private or self-signed certificates must configure their CA trust.
Disabling verification requires an explicit `setTLSOptions(false, ...)` (or
`"bverifypeer": false` in `vscpClientTcp` configuration) and is strongly
discouraged outside controlled tests: it permits credential theft by an
on-path attacker. Plain TCP selection is unchanged.

TLS verifies names as DNS hostnames. Use the certificate's DNS hostname for
verified TLS, rather than an IP-address endpoint.
Verified TLS supplies the hostname for SNI and verification. The explicitly
insecure opt-out omits hostname/SNI.

Local integration tests generate temporary certificates and exercise actual
TCP and TLS connections, including mutual TLS and verification failures:

```sh
cmake -S . -B build -DVSCP_BUILD_TESTS=ON
cmake --build build --target test_tcp_transport
```

These tests require Python 3 and the `openssl` command.

### Single-channel TCP receive loop

`vscpClientTcp` offers three text-mode connection choices:

| Mode | Configuration | Connections | Event reception |
| --- | --- | --- | --- |
| Default | `init(..., false)` | Two | Receive loop on a separate connection |
| Polling | `init(..., true)` | One | Polls for events |
| Single-channel receive loop | `init(..., false)` and `setSingleChannelReceiveLoop(true)` | One | Receives events asynchronously while sending commands |

The new mode is opt-in. Configure it, TLS settings, timeouts, and callbacks
before connecting. The receive-loop option cannot be combined with polling:
conflicting `init`, setter, or JSON settings are rejected. Disconnect before
changing the mode or other connection configuration. Applications must rebuild
against the changed C++ class layouts.

```cpp
#include <vscp-client-tcp.h>

int main()
{
    vscpClientTcp client;
    client.setTLSOptions(true, "/etc/vscp/ca.pem");
    client.setResponseTimeout(2000);
    if (client.init("stcp://daemon.example.org:9598", "user", "password", false) != VSCP_ERROR_SUCCESS ||
        client.setSingleChannelReceiveLoop() != VSCP_ERROR_SUCCESS ||
        client.connect() != VSCP_ERROR_SUCCESS) {
        return 1;
    }

    vscpEventEx outgoing = {};
    outgoing.vscp_class = 10;
    outgoing.vscp_type = 6;
    int sendResult = client.send(outgoing);

    vscpEventEx incoming = {};
    int receiveResult = client.receiveBlocking(incoming, 1000);
    // An incoming event is not necessarily an echo of the event just sent.
    int disconnectResult = client.disconnect();
    return (sendResult == VSCP_ERROR_SUCCESS &&
            (receiveResult == VSCP_ERROR_SUCCESS || receiveResult == VSCP_ERROR_TIMEOUT) &&
            disconnectResult == VSCP_ERROR_SUCCESS) ? 0 : 1;
}
```

The equivalent JSON settings can be passed to `initFromJson()` and are
preserved by `getConfigAsJson()`:

```json
{
  "host": "stcp://daemon.example.org:9598",
  "user": "user",
  "password": "password",
  "bpoll": false,
  "single-channel": true,
  "bverifypeer": true,
  "cafile": "/etc/vscp/ca.pem"
}
```

The server must support `CHID`, `RCVLOOP`, and commands during the receive
loop. In particular, command success replies must contain more than bare
`+OK`, for example `+OK - Success`: bare `+OK` is indistinguishable from a
receive-loop keepalive and is ignored. A command that receives no
unambiguous reply before the response timeout fails and closes the
connection to avoid matching a later reply to the wrong command. Call
`disconnect()` before reconnecting. Use the default mode or polling with a
server that cannot meet these requirements.

In this mode `send()` may run concurrently with queued receives and may also
be called from a receive callback; concurrent sends are serialized. Register
callbacks before connecting and do not change callback registration or other
connection configuration concurrently. Callbacks can begin before
`connect()` returns and run on the client's receive worker: keep them short,
and do not disconnect or destroy the client from a callback. `disconnect()`
called there returns `VSCP_ERROR_NOT_SUPPORTED`.
Registering or replacing a callback while this mode is active also returns
`VSCP_ERROR_NOT_SUPPORTED`; do not change callback activation flags while it
is running. Callbacks must not throw; a `std::exception` escaping a callback
stops the worker and is logged.
The shared interface returned by `getTcpReceive()` is for inspection only
while the client is running; direct operations can interfere with the reader.

With no callback active, `receive()` takes an event from a local queue and
returns `VSCP_ERROR_RCV_EMPTY` when it is empty. `receiveBlocking()` waits
for the supplied number of milliseconds, returns `VSCP_ERROR_TIMEOUT` on
expiry, and rejects negative timeouts. Both return
`VSCP_ERROR_NOT_SUPPORTED` while either callback is active. Events delivered
to callbacks are not queued. For `vscpEvent` receives, initialize the event
and release its data with `vscp_deleteEvent()` before reusing it.

`setfilter()` applies a local filter to future events; it does not change the
server-side filter. `getcount()` reports events already transferred to the
local queue, not events still in transit or buffered by the server, and
returns `VSCP_ERROR_OVERRUN` if the count cannot fit in `uint16_t`. The
queue is not bounded, so applications should consume events promptly.
An incoming event with more than `VSCP_MAX_DATA` (512) data bytes is
rejected and stops the receive worker with a read error.
`clear()` sends `CLRA` and discards locally buffered events after the server
acknowledges it; events arriving concurrently may also be discarded and
callbacks already in progress are not cancelled.

Only `SEND`, `NOOP`, `CLRA`, and `QUITLOOP` are supported by the underlying
connection during this receive loop. The wrapper's `getversion()`,
`getinterfaces()`, and `getwcyd()` return `VSCP_ERROR_NOT_SUPPORTED` in
single-channel mode. This is a text-mode feature; binary mode is separate.

## UDP and multicast addressing

`vscpClientUdp` and `vscpClientMulticast` support IPv4 and IPv6 destinations.
The existing defaults remain IPv4 broadcast `255.255.255.255:33333` for UDP and
group `224.0.23.158:9598` for multicast. An IPv6 multicast group must be chosen
explicitly.

The JSON `ip` value accepts an IPv4 address or hostname, a bracketed IPv6
address with an optional port, or an unbracketed IPv6 literal without a port.
For example:

```json
{"ip":"udp://[2001:db8::1]:40000"}
{"ip":"fe80::1234%eth0","port":40000}
{"ip":"udp://[ff02::114%25eth0]:9598","interface":"eth0","ttl":1}
```

In a `udp://` URI, `%25` encodes the `%` introducing an IPv6 zone. Bare `%`
zone identifiers are also accepted, and `getConfigAsJson()` returns bracketed
IPv6 URIs with the zone delimiter encoded as `%25`. The `port` property, when
present, overrides the port in `ip`; valid ports are 1 through 65535.

`interface` accepts an interface name or numeric index; the historical
`"name address"` form also uses its first token when the full name is not
found. Link-local IPv6 destinations and link-local multicast groups require an
interface, supplied either through `interface` or an address zone. If both are
present, they must identify the same interface. IPv6 unicast uses the selected
outgoing interface; multicast joins the group on it and uses it for outgoing
traffic. The local socket binds the configured port on the wildcard address.

For multicast, `ttl` specifies the IPv4 TTL or IPv6 hop limit (0-255). Local
multicast loopback remains disabled, as in the previous IPv4 implementation.
Invalid endpoints, ports, groups, interfaces, or conflicting zones fail
configuration or connection instead of falling back to broadcast.
Disconnect before changing a client's JSON configuration.

For encrypted datagrams, `encryption` selects AES-128, AES-192, or AES-256
and `key` contains exactly 32, 48, or 64 hexadecimal digits, respectively.
Filter fields are optional and can be configured independently of the key.
Invalid JSON settings are rejected without changing the existing configuration.

The client classes' layouts changed and `m_bRun` is now an atomic boolean;
C++ consumers should rebuild and should not bind that member to a `bool &`.
Real link-local and multicast integration tests and their optional isolated
Linux network setup are described in [tests/README.md](../tests/README.md).

## vscp_util

Purpose: shared utility functions such as base64, CRC, AES, MD5, config-file helpers, and password generation.

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

For network address matching, include `<vscp-socket-address.h>` to
construct the `union usa` passed to `vscp_parse_match_net`.

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
