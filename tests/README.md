# VSCP tests

The combined CMake project in this directory builds the common library and
client tests. It requires CMake 3.16 or newer, a C/C++ toolchain with C++17
support, Python 3, GoogleTest, and the OpenSSL, Expat, libcurl, and Mosquitto
development libraries. The TLS transport checks also require the `openssl`
command on `PATH`.

GoogleTest must be installed and discoverable by CMake. If your distribution
provides only its sources, build and install GoogleTest first, then set
`CMAKE_PREFIX_PATH` or `GTest_DIR` if needed. CMake downloads several other
source dependencies on the first configuration; see the
[library build instructions](../README.md#library-build-and-docs) for offline
source overrides.

The separate remote TCP/IP
[integration](vscpremotetcpip/alltest/README.md) and
[one-hour polling](vscpremotetcpip/longtest/README.md) suites are not part of
the combined test project and require a running daemon.

## Linux

On Debian or Ubuntu, install the build tools and dependencies:

```sh
sudo apt-get update
sudo apt-get install -y build-essential cmake python3 openssl \
  libgtest-dev libssl-dev libexpat1-dev libcurl4-openssl-dev libmosquitto-dev
```

From the repository root, configure, build, and run the combined tests:

```sh
cmake -S tests -B tests/build -DCMAKE_BUILD_TYPE=Debug
cmake --build tests/build --parallel
cmake --build tests/build --target test
```

## macOS

Install the Xcode Command Line Tools (`xcode-select --install`) if needed,
then install the test dependencies with Homebrew:

```sh
brew install cmake python googletest openssl@3 expat mosquitto cjson
```

From the repository root, configure CMake with the Homebrew prefixes, then
build and run the combined tests:

```sh
OPENSSL_PREFIX="$(brew --prefix openssl@3)"
EXPAT_PREFIX="$(brew --prefix expat)"
GTEST_PREFIX="$(brew --prefix googletest)"
MOSQUITTO_PREFIX="$(brew --prefix mosquitto)"
CJSON_PREFIX="$(brew --prefix cjson)"
export PATH="$OPENSSL_PREFIX/bin:$PATH"

cmake -S tests -B tests/build -DCMAKE_BUILD_TYPE=Debug \
  -DOPENSSL_ROOT_DIR="$OPENSSL_PREFIX" \
  -DCMAKE_PREFIX_PATH="$OPENSSL_PREFIX;$EXPAT_PREFIX;$GTEST_PREFIX;$MOSQUITTO_PREFIX;$CJSON_PREFIX" \
  -DCMAKE_CXX_FLAGS="-I$GTEST_PREFIX/include -I$CJSON_PREFIX/include" \
  -DCMAKE_EXE_LINKER_FLAGS="-L$GTEST_PREFIX/lib"
cmake --build tests/build --parallel
cmake --build tests/build --target test
```

## Running individual tests

The `test` target is a custom CMake target that runs the combined tests,
including the loopback transport and table checks. Some network tests may skip
when their required interfaces are unavailable. To build and run one test
instead on Linux or macOS, for example:

```sh
cmake --build tests/build --target unittest_guid
./tests/build/guid/unittest_guid
```

The push workflow also runs the metadata updater tests separately. They use
temporary local Git repositories and can be run from the repository root:

```sh
python3 -B tests/test_update_docs_metadata.py
```

## Windows

For an individual standalone test project, create a build directory inside its
source directory and configure it with Visual Studio and vcpkg:

```text
mkdir build
cd build
cmake .. -G "Visual Studio 16 2019" -DVCPKG_TARGET_TRIPLET=x64-windows -DCMAKE_TOOLCHAIN_FILE=C:\path\to\vcpkg\scripts\buildsystems\vcpkg.cmake
```

Then build the test with:

```text
cmake --build . --config Release
```

The executable is in the selected configuration's output directory, typically
`Release`. See each test project's CMake file for its executable name.

## UDP and multicast IPv6 integration tests

The UDP and multicast client tests exercise IPv4 and IPv6 loopback locally.
The separate-interface link-local, global unicast, and multicast tests skip
unless their test interface variables are set. On Linux, they can be run
without using the host network in an isolated user/network namespace (requires
`unshare` and `ip`):

```sh
cmake -S tests -B tests/build
cmake --build tests/build --target unittest_vscp_client_udp unittest_vscp_client_multicast
unshare -nUr sh -c '
  set -e
  ip link set lo up
  ip link add vscp0 type veth peer name vscp1
  ip link set vscp0 up
  ip link set vscp1 up
  ip -6 addr add fe80::1/64 dev vscp0 nodad
  ip -6 addr add fe80::2/64 dev vscp1 nodad
  ip -6 addr add 2001:db8:31::1/64 dev vscp0 nodad
  ip -6 addr add 2001:db8:31::2/64 dev vscp1 nodad
  ip -6 route add ff00::/8 dev vscp0
  ip -6 route add ff00::/8 dev vscp1 metric 2000
  export VSCP_TEST_MULTICAST_TX_INTERFACE=vscp0
  export VSCP_TEST_MULTICAST_RX_INTERFACE=vscp1
  export VSCP_TEST_LINKLOCAL_ADDRESS_A=fe80::1
  export VSCP_TEST_LINKLOCAL_ADDRESS_B=fe80::2
  export VSCP_TEST_GLOBAL_ADDRESS_A=2001:db8:31::1
  export VSCP_TEST_GLOBAL_ADDRESS_B=2001:db8:31::2
  tests/build/vscp-client-udp/unittest_vscp_client_udp \
    --gtest_filter=VscpClientUdp.Ipv6LinkLocalExchangeOnSeparateInterfaces:VscpClientUdp.Ipv6GlobalExchangeOnSeparateInterfaces
  tests/build/vscp-client-multicast/unittest_vscp_client_multicast \
    --gtest_filter=VscpClientMulticast.Ipv6MulticastExchangeOnSeparateInterfaces
'
```