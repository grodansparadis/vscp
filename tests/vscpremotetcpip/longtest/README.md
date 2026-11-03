# `vscpremotetcpif` long-running test

This GoogleTest executable contains a one-hour polling test for
`VscpRemoteTcpIf`. It opens a session to a running VSCP TCP/IP daemon, polls for
available events, receives each available event as a `vscpEventEx`, and closes
the session when the hour has elapsed. The test is intended for long-duration
connection and polling checks; it does not require a particular number of
events to arrive.

The endpoint and credentials are configured in `unittest.cpp`. A reachable
daemon and matching credentials are required. Run the test only against a
server for which you have authorization.

## Build and run

From the repository root, with CMake and GoogleTest available:

```sh
cmake -S tests/vscpremotetcpip/longtest -B build/vscpremotetcpip-longtest
cmake --build build/vscpremotetcpip-longtest
./build/vscpremotetcpip-longtest/unittest
```

GoogleTest must be discoverable by CMake (`find_package(GTest REQUIRED)`). On
Windows, run the generated `unittest.exe` from the selected build
configuration's output directory.

**Runtime:** the test runs for approximately one hour. Select it explicitly
when using a GoogleTest filter:

```sh
./build/vscpremotetcpip-longtest/unittest \
  --gtest_filter=VscpRemoteTcpIf.PollingTestExForOneHour
```
