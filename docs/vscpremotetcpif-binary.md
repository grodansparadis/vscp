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

## Versioned binary frame helpers

The helper-only framing API in `vscp-binary-protocol.h` supports a three-byte
prefix: `[0x55][version:3 | length:13, big-endian]`. The length excludes this
prefix and includes the entire inner frame. This is distinct from the raw
client transport's two-byte length prefix described above; the raw transport
does not negotiate or automatically enable authenticated framing.

Packet version **0** retains the existing inner frame format and is the default.
The existing `vscp_encryptFrame()` / `vscp_decryptFrame()` APIs are unchanged.
Packet version **1** requires AES-CBC (128, 192 or 256 bits), PKCS#7 padding,
and a separate 32-byte HMAC key. Its inner layout is:

```text
[type/encryption:1][ciphertext:N*16][IV:16][HMAC tag:16]
```

Encryption covers the body and CRC, not the type byte. After encryption,
HMAC-SHA-256 is computed over
`prefix || type/encryption || ciphertext || IV`; its leftmost 128 bits are
appended immediately after the IV. The receiver compares the tag in constant
time **before any decryption**, then validates padding and CRC. Failure leaves
the caller's output unchanged. Version 1 does not allow unencrypted frames.

`vscp_encryptFrameV1()` and `vscp_decryptFrameV1()` operate on inner frames,
without a prefix in their input/output buffers. They reconstruct the canonical
version 1 prefix for the MAC: `0x55` followed by the two-byte big-endian word
`(1 << 13) | innerFrameLength`, where the length includes the tag.
Inner frames must fit the 13-bit length. Callers must validate the received
prefix and require version 1 before calling the decryption helper.
They take a destination capacity and return the written
length, or zero on failure. Encryption needs at most 48 extra bytes;
decryption returns the unpadded length. Decryption always consumes the appended
IV and tag. These version 1 APIs use OpenSSL independently of the legacy AES
backend selector.

The HMAC backend is **OpenSSL**, not libsodium. The implementation calls
`HMAC(EVP_sha256(), ...)` and compares the first 16 digest bytes using
`CRYPTO_memcmp()`. No libsodium dependency is required.

For prefixed frames, use
`vscp_binary::encode(frame, encryption, aesKey, wire, 1, hmacKey)`.
On receipt, use `parseHeader(data, length, version)` and pass that version to
`decode(innerFrame, aesKey, plaintext, version, hmacKey)`. The two-argument
`parseHeader` accepts only legacy version 0, to prevent callers from losing
the authenticated-format version. Unknown versions are rejected.

Provision the AES and HMAC keys independently; never reuse the AES key as the
HMAC key. The MAC authenticates the prefix, type byte, ciphertext and IV,
but does not provide replay protection. Peers requiring authentication
must enforce version 1 and the agreed type/encryption policy rather than
silently accepting a downgrade to version 0.

### Authenticated framing example

The following functions operate on complete records, not arbitrary TCP chunks.
The sender's input is an inner plaintext frame including its type byte and CRC.
Keys must be provisioned independently; use a fresh random IV for each encryption
(the encoder does this automatically).

```cpp
#include <vscp-binary-protocol.h>

bool encodeAuthenticated(const std::vector<uint8_t>& plaintext,
                         const uint8_t* aesKey,  // 32 bytes for AES-256
                         const uint8_t* hmacKey, // independent 32-byte key
                         std::vector<uint8_t>& record)
{
    return vscp_binary::encode(plaintext, VSCP_ENCRYPTION_AES256,
                               aesKey, record, 1, hmacKey);
}

bool decodeAuthenticated(const std::vector<uint8_t>& record,
                         const uint8_t* aesKey,
                         const uint8_t* hmacKey,
                         std::vector<uint8_t>& plaintext)
{
    if (record.size() < vscp_binary::headerSize + 1) return false;
    size_t length = 0;
    uint8_t version = 0;
    if (!vscp_binary::parseHeader(record.data(), length, version) ||
        version != 1 || record.size() != vscp_binary::headerSize + length ||
        (record[vscp_binary::headerSize] & 0x0f) != VSCP_ENCRYPTION_AES256) {
        return false;
    }
    std::vector<uint8_t> inner(record.begin() + vscp_binary::headerSize, record.end());
    return vscp_binary::decode(inner, aesKey, plaintext, version, hmacKey);
}
```

Validate the decoded packet type and contents before dispatching. For stream
receivers, buffer exactly the indicated record length after validating the
header; reject unsupported versions rather than falling back to legacy decoding.
The returned plaintext retains the authenticated type/encryption byte.

### Deterministic interoperability vector

These public test keys and fixed IV are **only for testing**, not deployment.
The vector uses AES-256-CBC and was generated independently with OpenSSL's
`enc` command and Python's `hmac` module. Hex values are in wire byte order.

| Field | Hex |
| --- | --- |
| AES key | `000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f` |
| HMAC key | `202122232425262728292a2b2c2d2e2f303132333435363738393a3b3c3d3e3f` |
| Plain frame | `e000076de8` |
| Prefix | `552031` |
| Type/encryption | `e3` |
| Ciphertext | `b865bcf520d83d2708502ae712638aeb` |
| IV | `000102030405060708090a0b0c0d0e0f` |
| Full HMAC-SHA-256 | `270fec51e9794541f47aab3dd9f947cef382045953af73b7390175955ff993c7` |
| Appended tag | `270fec51e9794541f47aab3dd9f947ce` |

The plain body is command `0007` followed by CRC-CCITT `6de8`; PKCS#7 adds
twelve `0c` bytes before encryption. The prefix word `2031` means version 1,
inner length 49 (including the 16-byte tag). The complete record is 52 bytes:

```text
552031e3b865bcf520d83d2708502ae712638aeb000102030405060708090a0b0c0d0e0f270fec51e9794541f47aab3dd9f947ce
```

The `frameCryptoV1KnownAnswerAndTagConventions` test checks this exact record
and rejects the rightmost digest half, the former `IV || ciphertext` MAC order,
and tags calculated without the prefix. It also verifies that failed in-place
authentication leaves the encrypted input intact.

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
