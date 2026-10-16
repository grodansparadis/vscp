# Tests for the vscphelper.cpp

This is a testfile for the vscphelper file. 

The `frameCrypto*` and `binaryPacketVersions` tests cover legacy AES-CBC
compatibility, version 1 HMAC-SHA-256 interoperability, prefix version bits,
tampering, truncation, wrong keys, buffer sizes, and in-place operation.

From the repository root:

```sh
cmake -S tests -B build-tests
cmake --build build-tests --target unittest_vscphelper
./build-tests/vscphelper/unittest_vscphelper --gtest_filter='VscpHelper.frameCrypto*:VscpHelper.binaryPacketVersions'
```
