# Remote table interface test

Build the test against the VSCP libraries:

```sh
make
./build/test_tables_remote_class [host:port] [username] [password]
```

The CMake build requires Python 3. For automated success and failure checks
against a loopback server, run `make check`; no VSCP daemon is needed.

The manual test needs a running VSCP daemon and waits for Enter before exiting.
`BUILD_DIR` and `CMAKE_ARGS` can be passed to `make` to customize the build.
