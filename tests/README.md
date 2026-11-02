This folder contains testfiles for the VSCP & friends suite

test_vesphelper  - test for the tcp/if interface and the libvscphelper library code.
tables-tcp - Tests for tables creating, logging, handling in the tcp/ip interface.


Tests use GoogleTest which is part of this repository in the rootfoolder as googletest. 
GoogleTest must be configured and built before tests can be run.



To build

- Go to googletest folder
- Make folder *build* (*mkdir build*)
- Go to folder (**cd build**)
- Do **cmake ..**
- Do **make**
- Do **make test**

---

For new tests link with  

>-L../../googletest/build/lib -lgtest -lgtest_main 

and set include path

>_I ../../googletest/googletest/include

here with a test located in it's own folder under **tests**

See the `CMakeLists.txt` files under `tests/` for current test target definitions.

Use 

```
mkdir build
cd build
cmake .. -G "Visual Studio 16 2019" -DVCPKG_TARGET_TRIPLET=x64-windows  -DCMAKE_TOOLCHAIN_FILE=C:\Users\Administrator\Desktop\Development\vcpkg\scripts\buildsystems\vcpkg.cmake
```

in each test folder to create a makefile. Then do

```
cmake --build . --config Release
```

or 

```
msbuild libvscphelper.sln /p:Configuration=Release
```

to build the unittest.   Run the test with

```
./unittest
```

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