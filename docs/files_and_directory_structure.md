# Files and directory structure

The folders VSCP files are installed to has changed extensively from version 14.0. The reason for moving files to new install folders is to adopt to [lsb](https://refspecs.linuxfoundation.org/lsb.shtml) and other standards for Linux system that mandate where files should be installed.


## Source tree map

The repository is organized into a small set of top-level areas that separate the daemon, protocol definitions, libraries, tests, and documentation.

```text
vscp/
├── CMakeLists.txt
├── README.md
├── CHANGELOG
├── LICENSE
├── TODO.md
├── vcpkg.json
├── build/
├── build-tests/
├── cmake/
├── contrib/
├── database/
├── debug/
├── docs/
├── man/
├── resources/
├── service/
├── src/
│   ├── common/
│   └── vscp/
├── tests/
│   ├── clientlist/
│   ├── dm/
│   ├── drivers/
│   ├── helperlib/
│   ├── mdfparser/
│   ├── register/
│   ├── tcpiptls/
│   ├── testtools/
│   ├── userlist/
│   └── vscp-client*/
├── third-party/
├── tools/
└── .github/
```

This source tree is the development layout for the VSCP stack. The most important code lives under `src/`, while `tests/` contains unit and integration test projects and `docs/` holds the published documentation.

    
[filename](./bottom_copyright.md ':include')

