# Binary TCP/IP client

`VscpRemoteTcpIf` supports the text
[BINARY command](https://grodansparadis.github.io/vscp-doc-spec/#/./vscp_tcpiplink?id=tcpip-binary)
and [VSCP general binary protocol (VSCP-GBP) frames](https://grodansparadis.github.io/vscp-doc-spec/#/./vscp_over_binary).
Binary mode works over both TCP and TLS.

## Entering binary mode

Open and authenticate the connection normally, then call `doCmdBinary()`.
The method sends `BINARY\r\n` and switches mode only after receiving a
complete positive text response. `doCommand("BINARY\r\n")` uses the same
transition. The existing login is preserved.

```cpp
VscpRemoteTcpIf client;
if (VSCP_ERROR_SUCCESS != client.doCmdOpen("stcp://localhost:9598", "user", "password")) {
  return;
}
if (VSCP_ERROR_SUCCESS != client.doCmdBinary()) {
  client.doCmdClose();
  return;
}
```

Configure TLS trust using the existing TLS options before opening a connection.

`isBinaryMode()` reports the selected mode. Repeated `doCmdBinary()` calls
are harmless. A rejected command leaves the connection in text mode. A
negotiation timeout or malformed response closes the connection because the
peer's mode is then unknown. Entering binary mode during `RCVLOOP` is rejected.

Once binary mode is enabled, text commands return `VSCP_ERROR_PARAMETER`
without transmitting anything. This includes `NOOP`, `RETR`, `RCVLOOP`, and
arbitrary calls to `doCommand()`. Event send/receive methods use binary frames
instead, as described below. `doCmdClose()` closes the transport without sending
text `QUIT` or `QUITLOOP`; reopening a connection resets text mode.

The binary `TEXT` command is not a supported mode transition in this client.
Reconnect to return to text mode.

## TCP stream framing

The linked VSCP-GBP specification describes individual frames, not a TCP
record delimiter. This client uses the explicitly selected **two-byte
big-endian length prefix**. The peer must implement the same convention:

| Bytes | Contents |
| --- | --- |
| 0-1 | Length of the following VSCP-GBP frame, excluding the prefix |
| 2 | VSCP-GBP type/encryption byte |
| 3 onward | Frame body, CRC, and optional encryption padding/IV |

The length covers the entire frame, including its type byte, CRC, and any
PKCS#7 padding and appended IV. Lengths from 1 through 65535 are supported
by the raw transport API; zero lengths are invalid and close the connection.
Event and command formats impose their own additional size constraints.
There is no byte stuffing or CRLF delimiter.

Each receive reads one record, regardless of TCP packet boundaries.
Fragmented prefixes and payloads survive receive timeouts, and multiple
records received together remain separate. A receive timeout leaves the
caller's output unchanged. A truncated frame followed by disconnect returns
`VSCP_ERROR_STOPPED`. A send timeout closes the connection rather than
allowing a retry to duplicate a partially transmitted frame.

## Sending and receiving events

In binary mode the existing `doCmdSend()`, `doCmdSendEx()`, and
`doCmdSendLevel1()` methods write unencrypted binary event frames instead
of text `SEND` commands. The event helpers emit format 1 with nanosecond
timestamps and CRC-CCITT.

```cpp
vscpEventEx event = {};
event.head = VSCP_HEADER16_FRAME_VERSION_UNIX_NS;
event.timestamp_ns = vscp_makeTimeStampNs();
event.vscp_class = 10;
event.vscp_type = 6;
event.sizeData = 3;
event.data[0] = 0;
event.data[1] = 1;
event.data[2] = 35;
int result = client.doCmdSendEx(&event);
```

Sending returns after the frame has been written to the transport; it does
not wait for event confirmation. The event receive methods consume valid
event-confirm replies while waiting for an event and return a peer's nonzero
confirmation error.

`doCmdReceive()`, `doCmdReceiveEx()`, and `doCmdReceiveLevel1()` wait for an
asynchronous event using the configured response timeout. Both overloads of
`doCmdBlockingReceive()` use the supplied timeout, without requiring text
`RCVLOOP`. Incoming formats 0 and 1 are accepted, the embedded data size and
CRC are checked, and successful receives send a binary event-confirm reply
containing the event header and CRC.

```cpp
vscpEventEx received = {};
int result = client.doCmdBlockingReceive(&received, 1000);
```

To enable asynchronous events on a peer that requires binary `OPEN`, send
command code 7 and read its reply with the raw API first. The client does
not silently open a channel or change remote filtering settings.

## Commands, replies, and encrypted frames

`sendBinaryFrame()` and `receiveBinaryFrame()` transfer complete frame
vectors **without** the length prefix. They add/remove TCP framing, but
otherwise preserve the frame bytes. Use them for commands, replies,
encrypted events, or applications that manage event confirmation themselves.
They do not perform command dispatch, CRC validation, decryption, or automatic
confirmation. Do not pass text or a pre-prefixed frame to these methods.

```cpp
std::vector<uint8_t> command(5);
int result = vscp_writeCommandToFrame(command.data(), command.size(), 7, nullptr, 0);
if (VSCP_ERROR_SUCCESS == result) {
  result = client.sendBinaryFrame(command);
}
std::vector<uint8_t> reply;
if (VSCP_ERROR_SUCCESS == result) {
  result = client.receiveBinaryFrame(reply, 1000);
}
// Check reply type, command code, error code, and CRC before using its data.
```

Use `vscp_writeReplyToFrame()` to construct replies. These helpers include
the CRC in their output and calculate it over the body, excluding the type
byte and CRC slots.

For AES-CBC, call `vscp_encryptFrame()` with the agreed key and encryption
algorithm; it sets the frame type byte's encryption nibble. Allocate at least 32 extra bytes,
resize to its returned length, and pass the result to `sendBinaryFrame()`.
After receiving, call `vscp_decryptFrame()`, which validates PKCS#7 padding
and the decrypted frame's CRC, then validate its contents. With a null IV,
pass the complete inner frame length, including the appended IV; with an
explicit IV, exclude those 16 bytes from the length. The prefix remains
outside the encrypted frame. PKCS#7 is the only supported padding; UDP and
multicast configurations specifying `"padding": "zero"` are rejected.

The event convenience methods reject encrypted frames or unrelated command
replies. Use the raw API consistently when multiplexing commands and events
or handling encryption; route each received frame according to its type.
Serialize operations on each client instance, as with the text-mode API.

## Tests

From the repository root:

```sh
cmake -S tests -B build-tests
cmake --build build-tests --target test_tcp_transport
```

The loopback test server exercises text/binary transitions, text-command
rejection, event round trips and confirmations, CRC failures, raw commands
and replies, encrypted frames, TLS, fragmentation, coalescing, timeouts,
disconnects, and length limits.
