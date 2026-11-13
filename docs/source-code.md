# Source code guide

This guide is for developers who consume, embed, extend, or maintain the C,
C++, Python, and Node.js code under [`src/vscp`](../src/vscp/). It describes
the source tree as it exists in this repository, including components that are
not built by the top-level CMake project.

The protocol specification remains the authority for wire formats and protocol
behavior. See the [VSCP specification](https://grodansparadis.github.io/vscp-doc-spec/#/)
when source behavior and a protocol requirement need to be compared.

## Scope and compatibility

`src/vscp/common` contains the reusable native protocol and client
implementation. `src/vscp/python` and `src/vscp/nodejs` contain standalone
Level II API models; they are not native bindings to the C++ implementation.

The daemons, server interfaces (TCP/IP link server, client and user lists) and
command-line tools that used to live below `src/vscp` are no longer part of
this repository. The TCP/IP link server is maintained in
[vscp-tcpip-daemon](https://github.com/grodansparadis/vscp-tcpip-daemon) and
the MQTT daemon in
[vscp-mqtt-daemon](https://github.com/grodansparadis/vscp-mqtt-daemon).

The code has compatibility-sensitive C structures, C++ classes, driver entry
points, and serialized formats. When integrating it:

- preserve the layout of protocol and driver structures;
- use the event allocation and conversion helpers instead of copying an event's
  data pointer;
- treat numeric class, type, capability, error, and action constants as protocol
  values rather than local implementation details;
- do not assume that every installed header has an implementation in
  `vscp_common`; check the [build-target table](#cmake-targets);
- account for callbacks and receive queues being serviced by transport worker
  threads;
- use the VSCP error constants from `vscp.h` instead of interpreting all
  nonzero values as the same failure.

## Architecture at a glance

```text
Application
        |
        +-- CVscpClient transport API
        |      +-- TCP/TLS
        |      +-- MQTT
        |      +-- UDP or multicast
        |      +-- WebSocket v1/v2
        |      +-- CANAL or SocketCAN
        |
        |      +-- Level II driver
        |
        +-- Protocol model and helpers
               +-- vscpEvent / vscpEventEx
               +-- GUID, filters, date/time, units
               +-- serialization and measurement helpers
               +-- MDF, registers, and bootloaders
```

The reusable libraries are C++14 shared libraries with a small C-compatible
protocol surface. The primary external dependencies are OpenSSL, Expat, CURL,
Mosquitto, pthreads/Threads, nlohmann JSON, and spdlog. MDF parsing uses Expat
and CURL. MQTT transports use Mosquitto. Platform utility code supplies missing
POSIX APIs on Windows. The Python and Node.js packages do not link these
libraries.

## Build and consumption

The supported top-level library build is:

```sh
cmake -S . -B build
cmake --build build
cmake --install build --prefix /path/to/prefix
```

As a subproject:

```cmake
add_subdirectory(path/to/vscp)
target_link_libraries(my_application PRIVATE vscp_common)
```

Within the build tree the un-namespaced target name is `vscp_common`. Installed
exports use the `VSCP::` namespace:

```cmake
find_package(VSCP REQUIRED)
target_link_libraries(my_application PRIVATE VSCP::vscp_common)
```

### CMake targets

| Target | Implementation | Purpose |
|---|---|---|
| `vscp_all` | `vscp-all.cpp` plus every component target | Aggregate of all VSCP libraries, including GUID, MDF, and utility targets |
| `vscp_common` | `vscp-common.cpp` plus transitive client/core targets | Compatibility aggregate; the simplest general-purpose link target |
| `vscp_util` | Base64, MD5, CRC, AES, random passwords, configuration files, and helper logging (sources in `src/common`) | Low-level utilities used by every other target |
| `vscp_core` | Helpers, date/time, remote TCP interface, CANAL device interface, XML configuration, registers, HLO, and units | Non-connection protocol and model functionality |
| `vscp_guid` | `guid.cpp` | C++ GUID value type |
| `vscp_guidparser` | `vscp-guid-parser.c` | C GUID parser |
| `vscp_mdf` | `mdf.cpp` | MDF object model, parsing, and retrieval |
| `vscp_client_base` | `vscp-client-base.cpp`, `vscp-websocket.cpp` | Abstract client callback/filter support and the shared RFC 6455 WebSocket transport |
| `vscp_client_canal` | `vscp-client-canal.cpp` | Dynamically loaded CANAL driver client |
| `vscp_client_level2` | `vscp-client-level2.cpp` | Dynamically loaded Level II driver client |
| `vscp_client_tcp` | `vscp-client-tcp.cpp` | VSCP TCP/IP link client with selectable TLS |
| `vscp_client_udp` | `vscp-client-udp.cpp` | VSCP UDP client |
| `vscp_client_ws1` | `vscp-client-ws1.cpp` | WebSocket protocol version 1 client |
| `vscp_client_ws2` | `vscp-client-ws2.cpp` | WebSocket protocol version 2 client |
| `vscp_client_multicast` | `vscp-client-multicast.cpp` | VSCP multicast client |
| `vscp_client_mqtt` | `vscp-client-mqtt.cpp` | MQTT client and topic configuration |
| `vscp_client_socketcan` | `vscp-client-socketcan.cpp` | Linux-only SocketCAN client |

`vscp_client_socketcan` is built only on Linux and is then also linked into
`vscp_common` and `vscp_all`.

The install rule copies every `.h` and `.hpp` file in `src/vscp/common`, while
the target definitions compile only the sources listed above. Therefore,
**header installation is not a promise that the corresponding `.cpp` file is
part of `vscp_common`**. The following implementations are source-only in the
top-level build:

- `vscplevel2device.cpp`;
- the three `vscp-bootdevice*.cpp` implementations;
- `automation.cpp`, `mqtt.cpp`, and `udpsrv.cpp`, which are legacy daemon
  sources. They include headers (`controlobject.h`, `tcpipsrv.h`,
  `vscp_client_mqtt.h`) that are no longer in this repository and do not
  compile on their own.

Consumers of the bootloader and Level II device sources must add them, and
their dependencies, to their own build.

## Core data model

### Events

[`vscp.h`](../src/vscp/common/vscp.h) defines the wire-facing structures and
constants.

| Type | Payload storage | Typical use |
|---|---|---|
| `vscpEvent` | `pdata` points to a separately allocated payload | Queueing and APIs where variable allocation is acceptable |
| `vscpEventEx` | Fixed `data[VSCP_MAX_DATA]` member | Stack allocation, conversion, and bounded transport frames |
| `vscpEventFilter` | Priority, class, type, and GUID filter/mask pairs | Receive and publish filtering |
| `canalMsg` | Eight-byte CAN/CANAL frame | Level I and CAN driver interfaces |

Both event forms carry a header, origin/channel ID (`obid`), UTC time fields,
class, type, GUID, payload size, and payload. `vscpEvent` owns a separate
payload allocation; a shallow structure copy aliases that allocation and is not
a safe ownership transfer.

Use the helpers in [`vscphelper.h`](../src/vscp/common/vscphelper.h):

- `vscp_newEvent` / `vscp_deleteEvent_v2` for pointer-based events;
- `vscp_newEventEx` / `vscp_deleteEventEx` when an allocated extended event is
  required;
- `vscp_copyEvent` and `vscp_copyEventEx` for copies;
- `vscp_convertEventToEventEx` and `vscp_convertEventExToEvent` for conversion;
- the string, JSON, XML, binary, CAN ID, CRC, filter, and measurement helpers
  for protocol-safe serialization and interpretation.

`vscpEventEx` has embedded payload storage, but an allocated `vscpEventEx`
object still needs its matching delete helper. Event time is UTC. A frame
version in `head` selects the original split time fields or the Unix
nanosecond timestamp representation.

### GUIDs, classes, and types

[`guid.h`](../src/vscp/common/guid.h) provides the `cguid` C++ value type.
[`vscp-guid-parser.h`](../src/vscp/common/vscp-guid-parser.h) provides a C
parser. GUID byte zero is the most significant byte and byte 15 is the least
significant byte.

`vscp-class.h` and `vscp-type.h` are generated numeric definitions.
`vscp-hashclass.h` and `vscp-hashtype.h` are generated lookup definitions.
Do not hand-edit generated values; regenerate them from the protocol data used
by the VSCP project.

### Errors

Most client operations return `VSCP_ERROR_SUCCESS` (`0`) or one of the
`VSCP_ERROR_*` constants in `vscp.h`. Common distinctions include timeout,
empty receive queue, invalid parameter, not connected, unsupported operation,
authentication/authorization failure, parsing failure, invalid frame, and
transport/socket failure. Preserve the specific code when adapting this API to
exceptions or another error model.

### Client contract

[`vscp-client-base.h`](../src/vscp/common/vscp-client-base.h) defines
`CVscpClient`. All concrete clients expose the same main lifecycle:

1. configure with a transport-specific `init` method or `initFromJson`;
2. optionally configure filters, callbacks, and timeouts;
3. call `connect`;
4. send and receive `vscpEvent`, `vscpEventEx`, or `canalMsg`;
5. call `disconnect` before destruction.

Synchronous receives return an error when no event is available. Blocking
receives accept a timeout in milliseconds. A timeout of zero is documented by
the base API as no wait. Callbacks receive a reference valid for the callback
invocation and the caller-supplied opaque pointer. Do not retain that reference
without copying the event.

Concrete transports may have receive worker threads and internal queues.
Configure a client before connecting, and do not mutate connection or callback
configuration concurrently with active transport callbacks unless that
transport explicitly documents the operation as safe.

| Client | Header | Transport and notable behavior |
|---|---|---|
| `vscpClientTcp` | `vscp-client-tcp.h` | VSCP TCP/IP link; polling or separate send/receive connections; `tcp://` and `stcp://`; explicit TLS mode and certificate settings |
| `vscpClientMqtt` | `vscp-client-mqtt.h` | MQTT 3.x/5; publish/subscribe topic lists, QoS, retain, reconnect, TLS, will, user properties, and payload formats |
| `vscpClientUdp` | `vscp-client-udp.h` | UDP datagrams, interface/port configuration, encryption, filter, and receive queue |
| `vscpClientMulticast` | `vscp-client-multicast.h` | Multicast group membership, interface, TTL, encryption, and filtering |
| `vscpClientWs1` | `vscp-client-ws1.h` | First-generation VSCP WebSocket protocol |
| `vscpClientWs2` | `vscp-client-ws2.h` | Second-generation VSCP WebSocket protocol |
| `vscpClientCanal` | `vscp-client-canal.h` | Dynamically loads a Level I/CANAL driver and maps CAN frames to the common API |
| `vscpClientSocketCan` | `vscp-client-socketcan.h` | Linux SocketCAN interface with CAN filters and worker thread |
| `vscpClientLevel2` | `vscp-client-level2.h` | Dynamically loads a Level II driver |

Additional configuration references are available for
[CANAL](../src/vscp/common/vscp-client-canal.md),
[Level II](../src/vscp/common/vscp-client-level2.md), and
[MQTT](../src/vscp/common/vscp-client-mqtt.md).

## Protocol and model modules

Each row covers the named header and, where present, its same-named
implementation file.

| Files | Responsibility |
|---|---|
| `vscp.h` | Protocol structures, frame flags, limits, encryption IDs, interface IDs, and error codes |
| `canal.h`, `canal-macro.h` | CAN Abstraction Layer structures, status/statistics, flags, and conversion macros |
| `canaldlldef.h`, `level2drvdef.h` | Dynamic-library entry-point types for Level I and Level II drivers |
| `vscphelper.*` | Event lifecycle, serialization, conversion, filters, CRC, CAN mapping, measurements, and general protocol helpers |
| `guid.*`, `vscp-guid-parser.*` | C++ GUID operations and C parsing |
| `vscpdatetime.*` | UTC date/time parsing, formatting, validation, arithmetic, and current time |
| `vscpunit.*` | Measurement unit metadata and conversion support |
| `hlo.*` | High Level Object parsing and representation |
| `mdf.*` | Module Description File model, XML parsing, descriptions, registers, events, actions, firmware, media, and driver metadata |
| `register.*` | Standard/user register pages, remote access, abstractions for nodes, and MDF-associated register behavior |
| `canal-xmlconfig.*` | CANAL driver configuration wizard model, typed steps, choices, and bit-field configuration |
| `vscp-binary-protocol.h` | Binary framing helpers: headers, CRC, and frame plausibility checks |
| `vscp-websocket.*` | Minimal RFC 6455 WebSocket client used by the ws1 and ws2 clients |
| `vscp-client-datagram.h` | Shared socket and receive-queue support for the UDP and multicast clients |
| `vscp-socket-address.h` | Socket address parsing and formatting helpers |
| `vscp-udp-log.h` | Optional UDP debug logging, enabled by `VSCP_ENABLE_UDP_DEBUG` |
| `vscphelperlogging*` | Helper-library logging front end and backend forwarding |
| `automation.*` | Legacy daemon heartbeat and capability event generation; not buildable here |
| `vscpremotetcpif.*` | Lower-level VSCP TCP/IP command interface used by clients and older applications |
| `vscpcanaldeviceif.*` | Lower-level dynamically loaded CANAL device interface |
| `vscplevel2device.*` | Lower-level dynamically loaded Level II device interface; source-only in the top-level build |
| `vscpmqtt.h` | MQTT publish/subscribe configuration item types |
| `mqtt.*` | Legacy daemon MQTT worker adapter; not buildable here |
| `udpsrv.*` | Legacy daemon UDP listener; not buildable here |
| `vscp-bootdevice.*` | Bootloader base class, memory segments, firmware loading, and common state |
| `vscp-bootdevice-vscp.*` | Standard VSCP boot algorithm; source-only in the top-level build |
| `vscp-bootdevice-pic1.*` | PIC algorithm 1 bootloader; source-only in the top-level build |
| `actioncodes.h` | Decision Matrix action identifiers |
| `remotevariablecodes.h` | Decision Matrix remote-variable operation identifiers |
| `vscpd-caps.h` | Server/device capability bit definitions |
| `vscp-debug.h` | Debug category flags |
| `vscp-serial.h` | Serial framing and interface definitions |
| `version.h` | Shared source version definitions |
| `vscp-common.cpp`, `vscp-all.cpp` | Empty translation units used to create the aggregate targets |

## Python and Node.js Level II models

The language packages mirror the shape of the C++ `vscpClientLevel2` API, but
their current implementations are **in-process models for application
development, testing, and simulation**. They do not use `ctypes`, CFFI, a
Node.js native addon, sockets, or another bridge to load a Level II driver.

In both implementations, `connect` creates local connection state rather than
opening the configured interface. `send` validates and records or emits an
event without writing to a device. `receive` reads only events supplied through
`inject_event`/`injectEvent`. Version, interface, and capability responses are
local values. A third-party application that needs real device I/O must supply
a native/transport adapter or use the C++ `vscpClientLevel2` implementation.

### Python

[`src/vscp/python`](../src/vscp/python/) is an independent Python 3.8+ package
named `vscp-level2-client`.

| Files | Responsibility |
|---|---|
| `vscp_level2/client.py` | `VSCPLevel2Client`, event/filter dataclasses, constants, local queue, statistics, JSON configuration, and simulation injection |
| `vscp_level2/__init__.py` | Public exports, aliases, and factory functions |
| `pyproject.toml` | Package metadata, setuptools configuration, Python requirement, and optional development tools |
| `README.md` | Python API and examples |
| `examples/*.py` | Basic use, event handling, and filtering demonstrations |
| `tests/test_level2_client.py` | Async package behavior tests |

The queue is a bounded `queue.Queue` protected by an `RLock` for state and
statistics. `receive` is declared async but performs the blocking
`queue.Queue.get` call; callers should not expect other tasks on the same event
loop to make progress during that wait.

### Node.js

[`src/vscp/nodejs`](../src/vscp/nodejs/) is an independent CommonJS package
named `vscp-level2-client` for Node.js 14+.

| Files | Responsibility |
|---|---|
| `lib/vscp-level2-client.js` | `EventEmitter` client, event/filter classes, constants, local queue, statistics, JSON configuration, and simulation injection |
| `lib/index.js` | Package exports, aliases, factories, version, and package information |
| `types/index.d.ts` | TypeScript declarations for the CommonJS API |
| `package.json` | Package entry points, scripts, supported Node version, and published file set |
| `README.md` | Node.js API and examples |
| `examples/*.js` | Basic use, event handling, filtering, and package behavior test |

`injectEvent` appends to the bounded array queue and emits `newEvent`.
`receive` either removes an already queued event or waits for `newEvent` until
its timeout. Consumers should register an `error` listener because Node.js
`EventEmitter` treats an unhandled `error` event as an exception.

## Windows compatibility layer

[`src/vscp/common/win32helpers`](../src/vscp/common/win32helpers/) supplies
small POSIX compatibility declarations and implementations used by the common
code on Windows.

| Files | Responsibility |
|---|---|
| `getopt.c`, `getopt_long.c`, `getopt.h` | POSIX-style command-line option parsing |
| `gettimeofday.c`, `gettimeofday.h`, `sys/time.h` | Time compatibility |
| `websock-w32.c`, `websock-w32.h`, `sys/socket.h`, `netdb.h` | Socket, polling, and WebSocket support declarations |
| `unistd.h`, `strings.h`, `syslog.h` | POSIX function and logging compatibility |

Keep these headers platform-gated. They intentionally shadow POSIX header
names for Windows builds and should not precede native Unix headers on Unix
include paths.

## Threading and lifetime rules

The code uses `pthread_t`, mutexes, semaphores, `std::thread`, and transport
library callbacks. The exact primitive varies by component because the source
spans multiple generations.

- A `m_bQuit`, `m_quit`, or equivalent flag normally requests worker
  termination; the owner must then join or otherwise stop the worker using that
  component's lifecycle method.
- Client receive queues are paired with a mutex and usually a semaphore.
  Lock only around queue/list mutation and never invoke external transport code
  while holding a shared collection lock unless the existing implementation
  requires it.
- Callback data pointers are non-owning and must remain valid until callbacks
  are disabled and the client is disconnected.
- Dynamically loaded driver function pointers are valid only while the driver
  library remains loaded.
- `vscpEvent::pdata` and queued `vscpEvent *` values require explicit helper
  cleanup; `vscpEventEx` payload storage is inline.

## Extending the code

### Add a client transport

1. derive from `CVscpClient`;
2. implement all send/receive overloads and lifecycle methods;
3. preserve VSCP error codes and timeout units;
4. implement `getConfigAsJson` and `initFromJson` symmetrically;
5. apply `vscpEventFilter` before exposing received events;
6. dispatch callbacks without leaking transport-owned event references;
7. add a focused CMake target and link it into `vscp_common` only if it belongs
   in the compatibility aggregate;
8. add transport tests under `tests/vscp-client-*`.

### Add a driver type

Prefer the existing Level I/CANAL or Level II ABI. Keep exported function
signatures exactly aligned with `canaldlldef.h` or `level2drvdef.h`. Driver
handles, blocking semantics, queue ownership, and GUID translation are part of
the integration contract.

### Add protocol constants

Update the authoritative protocol data and regenerate class/type/hash headers.
For manual capability, action, or error additions, preserve assigned numeric
values and update the protocol specification and serialization tests.

## Source index

This index accounts for every production C/C++ source file under
`src/vscp/common`.
Files with the same base name are grouped as an implementation/header pair.
The production Python and Node.js files are indexed separately below.

### Common protocol and infrastructure

```text
actioncodes.h
automation.cpp / automation.h
canal-macro.h
canal-xmlconfig.cpp / canal-xmlconfig.h
canal.h
canaldlldef.h
guid.cpp / guid.h
hlo.cpp / hlo.h
level2drvdef.h
mdf.cpp / mdf.h
mqtt.cpp / mqtt.h
register.cpp / register.h
remotevariablecodes.h
udpsrv.cpp / udpsrv.h
version.h
vscp-all.cpp
vscp-binary-protocol.h
vscp-bootdevice-pic1.cpp / vscp-bootdevice-pic1.h
vscp-bootdevice-vscp.cpp / vscp-bootdevice-vscp.h
vscp-bootdevice.cpp / vscp-bootdevice.h
vscp-class.h
vscp-client-base.cpp / vscp-client-base.h
vscp-client-canal.cpp / vscp-client-canal.h
vscp-client-datagram.h
vscp-client-level2.cpp / vscp-client-level2.h
vscp-client-mqtt.cpp / vscp-client-mqtt.h
vscp-client-multicast.cpp / vscp-client-multicast.h
vscp-client-socketcan.cpp / vscp-client-socketcan.h
vscp-client-tcp.cpp / vscp-client-tcp.h
vscp-client-udp.cpp / vscp-client-udp.h
vscp-client-ws1.cpp / vscp-client-ws1.h
vscp-client-ws2.cpp / vscp-client-ws2.h
vscp-common.cpp
vscp-debug.h
vscp-guid-parser.c / vscp-guid-parser.h
vscp-hashclass.h
vscp-hashtype.h
vscp-serial.h
vscp-socket-address.h
vscp-type.h
vscp-udp-log.h
vscp-websocket.cpp / vscp-websocket.h
vscp.h
vscpcanaldeviceif.cpp / vscpcanaldeviceif.h
vscpd-caps.h
vscpdatetime.cpp / vscpdatetime.h
vscphelper.cpp / vscphelper.h
vscphelperlogging-forward.cpp
vscphelperlogging-internal.h
vscphelperlogging.cpp / vscphelperlogging.h
vscplevel2device.cpp / vscplevel2device.h
vscpmqtt.h
vscpremotetcpif.cpp / vscpremotetcpif.h
vscpunit.cpp / vscpunit.h
```

### Windows helpers

```text
win32helpers/getopt.c
win32helpers/getopt.h
win32helpers/getopt_long.c
win32helpers/gettimeofday.c
win32helpers/gettimeofday.h
win32helpers/netdb.h
win32helpers/strings.h
win32helpers/syslog.h
win32helpers/sys/socket.h
win32helpers/sys/time.h
win32helpers/unistd.h
win32helpers/websock-w32.c
win32helpers/websock-w32.h
```

### Python package

```text
python/pyproject.toml
python/vscp_level2/__init__.py
python/vscp_level2/client.py
python/udp_debug_receiver.py
```

### Node.js package

```text
nodejs/package.json
nodejs/lib/index.js
nodejs/lib/vscp-level2-client.js
nodejs/lib/vscp-guid.js
nodejs/types/index.d.ts
nodejs/vscp-guid/package.json
nodejs/vscp-guid/lib/vscp-guid.js
nodejs/vscp-guid/types/index.d.ts
```

## Where to start

| Goal | Start with |
|---|---|
| Send or receive events | `vscp-client-base.h`, then the chosen `vscp-client-*.h` |
| Understand an event | `vscp.h`, `vscphelper.h`, `guid.h` |
| Parse device metadata | `mdf.h` |
| Read/write device registers | `register.h` |
| Implement a CANAL driver | `canal.h`, `canaldlldef.h`, `canal-macro.h` |
| Implement a Level II driver | `vscp.h`, `level2drvdef.h` |
| Talk to a Level II driver | `vscp-client-level2.h`, then `vscp-client-level2.md` |
| Update device firmware | `vscp-bootdevice.h`, then `vscp-bootdevice-vscp.h` |
| Use the Python simulation API | `python/README.md`, then `python/vscp_level2/client.py` |
| Use the Node.js simulation API | `nodejs/README.md`, then `nodejs/lib/vscp-level2-client.js` |
