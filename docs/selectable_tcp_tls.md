# Selectable TLS for TCP/IP

This repository supports selectable TLS for both the TCP/IP client classes and the daemon TCP/IP server.

## Client side (`VscpRemoteTcpIf`, `vscpClientTcp`)

TLS selection mode is explicit and can be one of:

- `auto` (default): use `stcp://` for TLS and `tcp://` (or no prefix) for plain TCP.
- `force-tls`: always use TLS even if endpoint uses `tcp://`.
- `force-plain`: never use TLS even if endpoint uses `stcp://`.

### Prefix behavior in `auto` mode

- `stcp://host:port` => TLS
- `tcp://host:port` => plain TCP
- `host:port` => plain TCP

In `auto` mode, each `doCmdOpen()` call resolves transport from the current endpoint prefix. TLS state does not stick between calls.

## Server side (`tcpipsrv`)

Secure and non-secure listening sockets are selected by the listening port string (from `sockettcp` parsing):

- `9598` => plain TCP socket
- `9598s` => TLS socket
- `9598,9599s` => mixed plain + TLS sockets

TLS context is initialized only when at least one secure (`s`-suffixed) listening endpoint is configured. If a secure endpoint is configured without a TLS certificate, startup fails with an explicit error.
