# Files and directory structure

The folders VSCP files are installed to has changed extensively from version 14.0. The reason for moving files to new install folders is to adopt to [lsb](https://refspecs.linuxfoundation.org/lsb.shtml) and other standards for Linux system that mandate where files should be installed.


## Source tree map

The repository is organized into a small set of top-level areas that separate the daemon, protocol definitions, libraries, tests, and documentation.

```text
vscp/
├── CMakeLists.txt    - The top-level CMake build script for the VSCP project.
├── README.md         - The top-level README file for the VSCP project.
├── LICENSE           - The license file for the VSCP project.
├── vcpkg.json        - The vcpkg manifest file for managing dependencies.
├── build/            - The build output directory for the VSCP project.
├── build-tests/      - The build output directory for the test projects.
├── cmake/            - CMake modules and scripts used for building the project.
├── contrib/          - Contributed scripts, tools, or third-party integrations.
├── docs/             - Documentation for the VSCP project.
├── man/              - Manual pages for the VSCP project.
├── src/              - The source code for the VSCP project, including common code and the main VSCP components.
│   ├── common/       - Common code non specific to VSCP shared across different parts of the VSCP project.
│   └── vscp/         - The main VSCP common components and implementation code.
├── tests/            - Unit and integration test projects for the VSCP project.
│   ├── clientlist/   - Code for client handling common to user code.
│   ├── drivers/      - Code for various hardware drivers.
│   ├── helperlib/    - Helper library code used across different test projects.
│   ├── mdfparser/    - Code for parsing module description files (MDF).
│   ├── register/     - Code for handling register operations.
│   ├── tcpiptls/     - Code for TCP/IP and TLS communication tests.
│   ├── testtools/    - Utility tools for testing purposes.
│   ├── userlist/     - Code for managing user lists.
│   └── vscp-client*/ - Code for testing the VSCP client.
├── third-party/      - Third-party libraries and dependencies used by the VSCP project.
├── tools/           - Utility tools and scripts for the VSCP project.
└── .github/         - GitHub-specific configuration files, workflows, and actions for the VSCP project.
```

This source tree is the development layout for the VSCP stack. The most important code lives under `src/`, while `tests/` contains unit and integration test projects and `docs/` holds the published documentation.

    
[filename](./bottom_copyright.md ':include')

