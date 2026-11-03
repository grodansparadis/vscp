# `vscpremotetcpif` integration tests

This GoogleTest executable exercises the `VscpRemoteTcpIf` client against a
running VSCP TCP/IP daemon. These are integration tests, not offline unit tests:
they require network access to the endpoint configured in `unittest.cpp`.

## What is covered

The suite checks:

- Opening and closing a session, sending `NOOP`, and reporting connection state
  and protocol level.
- Connection failures for an unknown host and invalid user or password input.
- Receiving events through both `vscpEvent` and `vscpEventEx` APIs after
  sending test events from a second session.
- Polling for daemon-generated traffic and receiving in the receive loop.

The event tests send VSCP control `TURNON` events (zone `0x55`, subzone `0xAA`)
to the configured daemon. Run them only against a test server for which you
have authorization; do not use a production or shared server unless sending
these events is explicitly approved. The server must have credentials that
match the values configured in `unittest.cpp`. The connection and traffic tests
depend on the daemon being available and, for polling/receive-loop tests,
generating events.

## Build and run

From the repository root, with CMake and GoogleTest available:

```sh
cmake -S tests/vscpremotetcpip/alltest -B build/vscpremotetcpip-alltest
cmake --build build/vscpremotetcpip-alltest
./build/vscpremotetcpip-alltest/unittest
```

GoogleTest must be discoverable by CMake (`find_package(GTest REQUIRED)`). On
Windows, run the generated `unittest.exe` from the selected build
configuration's output directory.

Individual cases can be selected with GoogleTest filters, for example:

```sh
./build/vscpremotetcpip-alltest/unittest \
  --gtest_filter=VscpRemoteTcpIf.ConnectVscp2HostUserPassword
```

The polling and receive-loop cases can take up to about 70 seconds when the
daemon is not producing the expected events.
