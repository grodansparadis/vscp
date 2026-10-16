# Tests for the vscphelper.cpp

This is a testfile for the vscphelper file. 

The `frameCrypto*` and `binaryPacketVersions` tests cover legacy AES-CBC
compatibility, version 1 HMAC-SHA-256 interoperability, prefix version bits,
tampering, truncation, wrong keys, buffer sizes, and in-place operation.

`frameCryptoV1KnownAnswerAndTagConventions` additionally locks down a fixed
AES-256-CBC/HMAC-SHA-256 wire vector generated independently using OpenSSL and
Python. It rejects rightmost-half truncation, incorrect MAC byte order, and
prefix omission, while checking that failed in-place decryption is non-mutating.
The vector and complete-record usage examples are documented in
[Versioned binary frame helpers](../../docs/vscpremotetcpif-binary.md#versioned-binary-frame-helpers).
The implementation uses OpenSSL for both HMAC and constant-time comparison;
libsodium is not required.

From the repository root:

```sh
cmake -S tests -B build-tests
cmake --build build-tests --target unittest_vscphelper
./build-tests/vscphelper/unittest_vscphelper --gtest_filter='VscpHelper.frameCrypto*:VscpHelper.binaryPacketVersions'
```
