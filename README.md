<h1>VSCP & Friends</h1>

![License](https://img.shields.io/badge/license-MIT-blue.svg)
[![Build VSCP library on Linux](https://github.com/grodansparadis/vscp/actions/workflows/build.yml/badge.svg)](https://github.com/grodansparadis/vscp/actions/workflows/build.yml)
[![Build VSCP library on Windows](https://github.com/grodansparadis/vscp/actions/workflows/build-windows.yml/badge.svg)](https://github.com/grodansparadis/vscp/actions/workflows/build-windows.yml)
[![Build VSCP library on macOS](https://github.com/grodansparadis/vscp/actions/workflows/build-macos.yml/badge.svg)](https://github.com/grodansparadis/vscp/actions/workflows/build-macos.yml)
[![Release](https://img.shields.io/github/release/grodansparadis/vscp.svg)](https://github.com/grodansparadis/vscp/releases)
[![Project Status: Active – The project has reached a stable, usable state and is being actively developed.](https://www.repostatus.org/badges/latest/active.svg)](https://www.repostatus.org/#active)

![VSCP Logo](https://github.com/grodansparadis/vscp-logo/blob/master/logo_100.png)

*You can look at each device individually and create control software for each one, but we took a different approach: we designed a black-box device model and built control software that works with everything that exists. __One to unite them all__.*


<p>VSCP (Very Simple Control Protocol) is a framework for IoT and M2M tasks. The framework defines common methods for device discovery, configuration, interfacing with remote devices, and updating firmware on devices built on different architectures. A daemon/server is available that runs on many platforms and provides a web server, websocket, REST, driver, MQTT, and TCP/IP interface with SSL security.

  * Documentation for different parts of VSCP is available here: [https://docs.vscp.org](https://docs.vscp.org)
  * A list of all available VSCP packages is available on the [project homepage](https://vscp.org)
  * The MQTT daemon has been separated from this package and is now available here: [https://github.com/grodansparadis/vscp-mqtt-daemon](https://github.com/grodansparadis/vscp-mqtt-daemon)
  * The TCP/IP daemon has been separated from this package and is now available here: [https://github.com/grodansparadis/vscp-tcpip-daemon](https://github.com/grodansparadis/vscp-tcpip-daemon)

The code in this repository is mainly used as a common library for different VSCP components. It can be included in other projects to provide core functionality and facilitate communication between various VSCP modules. Typically, it is used as a Git submodule, but it can also be included directly as a library.


Clone with:

```bash
git clone -j4 --recurse-submodules https://github.com/grodansparadis/vscp.git
```

<p>A short introduction to VSCP is available
<a href="https://www.slideshare.net/keHedman/2014-01-33087344">here</a> and
<a href="https://www.slideshare.net/keHedman/vscp-presentation-eindhoven">here</a>.</p>
</p>

<p>
Also there is a <a href="https://github.com/grodansparadis/vscp_firmware"firmware repository</a> with many examples for different platforms and a
<a href="https://github.com/grodansparadis/vscp_html5">HTML5 websocket UI repository</a>.
</p>

## Library build and docs

The project builds a set of shared VSCP libraries. The aggregate target `vscp_all` links the full stack, while the individual component libraries can be linked separately for smaller builds.

```bash
cmake -S . -B build
cmake --build build --target vscp_all
```

See the library reference at [docs/library_reference.md](docs/library_reference.md) for a library-by-library summary and usage examples.

## License

The entire source code is published under the MIT license. Please also consider the licenses of any third-party libraries used by the project.

## Contribution

Unless you explicitly state otherwise, any contribution intentionally submitted for inclusion in the work by you shall be licensed as above, without any additional terms or conditions.


## Thanks
We would like to express our sincere thanks to our [sponsors](https://github.com/sponsors/grodansparadis) and to [vscp.org](https://vscp.org). Please support them. Every contribution is valuable, and their support has helped this project greatly.

We would also like to thank the following open-source projects on which much of this code is built:

* The excellent [nlohmann json parser](https://github.com/nlohmann/json)
* The excellent [libexpat XML library](https://github.com/libexpat/libexpat)
* The excellent [mosquitto library](https://github.com/eclipse/mosquitto)
* The excellent [spdlog library](https://github.com/gabime/spdlog)
* The excellent [civetweb project](https://github.com/civetweb/civetweb)
* The excellent [mustache library](https://github.com/janl/mustache.js)
* All other code used to build this project.

_Great things are built on the shoulders of giants. Thank you!_

Copyright (C) 2000-2026 Ake Hedman and contributors, the VSCP project — MIT license.

