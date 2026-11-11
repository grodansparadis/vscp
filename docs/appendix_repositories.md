# Appendix: VSCP repositories

*As of 2026-11-11.*

The grodansparadis GitHub account holds 217 public repositories, nearly all part of VSCP (Very Simple Control Protocol), an open framework for IoT and automation. 76 of them were pushed in 2026. Descriptions come from each repository's README.

## At a glance

| Group | Repos | Pushed in 2026 |
| --- | --- | --- |
| Core platform and tools | 31 | 10 |
| Specifications and documentation | 10 | 1 |
| Level II drivers | 28 | 10 |
| Level I (CANAL) drivers | 21 | 2 |
| Python libraries and scripts | 18 | 7 |
| Node.js, Node-RED and Arduino | 15 | 9 |
| CAN4VSCP hardware modules | 26 | 24 |
| DIN, P1 and other boards | 25 | 5 |
| Demos and examples | 29 | 6 |
| Forks and non-VSCP | 14 | 2 |
| **Total** | **217** | **76** |

13 repositories are archived and 12 are forks. Each group below lists its repositories newest first.

## Core platform and tools (31)

Daemons, libraries, configuration tools and infrastructure that every other repository builds on.

| Repository | What it is | Last push |
| --- | --- | --- |
| [vscp-mqtt-daemon](https://github.com/grodansparadis/vscp-mqtt-daemon) | VSCP daemon that exposes events over MQTT; sibling of the tcp/ip daemon. | 2026-11 |
| [vscp-tcpip-daemon](https://github.com/grodansparadis/vscp-tcpip-daemon) | Central VSCP server exposing a TLS-capable tcp/ip link interface; hosts level I and II drivers. | 2026-11 |
| [vscp](https://github.com/grodansparadis/vscp) | Main VSCP project repo: protocol overview, black-box device model, library build and docs. | 2026-11 |
| [vscp-helper-lib](https://github.com/grodansparadis/vscp-helper-lib) | Shared helper library for VSCP (Linux, Raspberry Pi, Windows, macOS); wrapped for Python and Node.js. | 2026-11 |
| [sockettcp](https://github.com/grodansparadis/sockettcp) | MIT-licensed tcp/ip client/server C library with TLS, lifted from Civetweb. **Archived.** | 2026-10 |
| [vscp-works-qt](https://github.com/grodansparadis/vscp-works-qt) | VSCP Works: Qt rewrite of the configuration and diagnostics tool for VSCP networks. | 2026-10 |
| [vscp-firmware](https://github.com/grodansparadis/vscp-firmware) | Reference firmware for implementing VSCP on many hardware architectures. | 2026-10 |
| [vscp-samples](https://github.com/grodansparadis/vscp-samples) | Assorted examples for VSCP and related tools. | 2026-09 |
| [vscp-cli](https://github.com/grodansparadis/vscp-cli) | Command-line tool to send and receive events from a VSCP daemon or device (work in progress). | 2026-08 |
| [vscp-simulator](https://github.com/grodansparadis/vscp-simulator) | Simulates a VSCP level I/II node for testing bootloaders and node behaviour. | 2026-02 |
| [vscp-classes](https://github.com/grodansparadis/vscp-classes) | Source specification of VSCP classes and types; scripts generate docs and code from it. | 2025-02 |
| [vscp-pic1-bootloader](https://github.com/grodansparadis/vscp-pic1-bootloader) | PIC18F bootloader for VSCP over CAN (PIC1 algorithm in VSCP Works). | 2025-02 |
| [vscpworks](https://github.com/grodansparadis/vscpworks) | Original VSCP Works tool, deprecated and replaced by vscp-works-qt. **Archived.** | 2025-02 |
| [vscp-js](https://github.com/grodansparadis/vscp-js) | Core JavaScript libraries for the VSCP event bus and daemon variables. | 2024-11 |
| [vscp-maintenance-interface](https://github.com/grodansparadis/vscp-maintenance-interface) | Symfony web interface for maintaining VSCP-related databases. **Archived.** | 2024-04 |
| [vscp-mv](https://github.com/grodansparadis/vscp-mv) | Web tool to visualise and publish VSCP measurement data. **Archived.** | 2024-03 |
| [vscp-ux](https://github.com/grodansparadis/vscp-ux) | HTML5 UI component framework and websocket demos for VSCP. **Archived.** | 2022-06 |
| [vscp-mdf-parser-lib](https://github.com/grodansparadis/vscp-mdf-parser-lib) | Parser library for Module Description Files (MDF). | 2022-01 |
| [vscp-logo](https://github.com/grodansparadis/vscp-logo) | Official VSCP logo (CC BY). | 2021-03 |
| [vscp-admin](https://github.com/grodansparadis/vscp-admin) | No README; likely an admin interface for VSCP. | 2020-02 |
| [vscp-labview](https://github.com/grodansparadis/vscp-labview) | Contributed LabVIEW code for VSCP. | 2019-10 |
| [vscp-install-windows](https://github.com/grodansparadis/vscp-install-windows) | Windows installer build for VSCP. **Archived.** | 2019-10 |
| [linuxvscpl1](https://github.com/grodansparadis/linuxvscpl1) | VSCP interface library for level I on Linux. | 2019-10 |
| [vscp-relayd](https://github.com/grodansparadis/vscp-relayd) | Relays a level I/CANAL driver to a VSCP daemon over the VSCP link protocol. | 2019-10 |
| [uvcpd](https://github.com/grodansparadis/uvcpd) | Micro VSCP daemon (work in progress). | 2019-10 |
| [vscp-helper-csharp](https://github.com/grodansparadis/vscp-helper-csharp) | Unfinished contributed C# wrapper for the helper library. | 2019-10 |
| [vscp-install-deb](https://github.com/grodansparadis/vscp-install-deb) | Scripts for building VSCP Debian packages. | 2019-09 |
| [vscp-graphics](https://github.com/grodansparadis/vscp-graphics) | No README; graphics assets for VSCP. | 2018-09 |
| [vscp-php](https://github.com/grodansparadis/vscp-php) | PHP utilities and database handling for VSCP. | 2018-03 |
| [vscp-openwrt](https://github.com/grodansparadis/vscp-openwrt) | Build notes and feeds for running VSCP on OpenWrt. | 2017-10 |
| [vscp-test-simpleui](https://github.com/grodansparadis/vscp-test-simpleui) | Simple phone and tablet UI test for VSCP. | 2014-09 |

## Specifications and documentation (10)

Specs and manuals, mostly published as books from these sources.

| Repository | What it is | Last push |
| --- | --- | --- |
| [vscp-doc-spec](https://github.com/grodansparadis/vscp-doc-spec) | The official VSCP protocol specification. | 2026-10 |
| [vscp-doc-firmware](https://github.com/grodansparadis/vscp-doc-firmware) | The official VSCP firmware specification. | 2025-09 |
| [vscp-doc-canal](https://github.com/grodansparadis/vscp-doc-canal) | Specification of CANAL, the CAN abstraction layer used by level I drivers. | 2025-02 |
| [vscp-doc-vscpworks](https://github.com/grodansparadis/vscp-doc-vscpworks) | User documentation for VSCP Works. | 2022-05 |
| [vscp-doc-nodejs](https://github.com/grodansparadis/vscp-doc-nodejs) | node.js API documentation for VSCP. | 2022-05 |
| [vscp-doc-helper-library](https://github.com/grodansparadis/vscp-doc-helper-library) | Documentation for the VSCP helper library. | 2022-05 |
| [vscp-doc-vscpd](https://github.com/grodansparadis/vscp-doc-vscpd) | Documentation for the legacy VSCP daemon. **Archived.** | 2021-05 |
| [vscp-doc-pyvscp](https://github.com/grodansparadis/vscp-doc-pyvscp) | Python API documentation for VSCP. **Archived.** | 2020-11 |
| [vscp-doc-js](https://github.com/grodansparadis/vscp-doc-js) | Documentation for the core JavaScript libraries. **Archived.** | 2020-11 |
| [vscp-doc-mv](https://github.com/grodansparadis/vscp-doc-mv) | No README; documentation for vscp-mv. **Archived.** | 2018-04 |

## Level II drivers (28)

Plug-in drivers loaded by the VSCP daemon (Linux, Windows, macOS). Recent work targets daemon v14; the 2019 batch predates it.

| Repository | What it is | Last push |
| --- | --- | --- |
| [vscpl2drv-template](https://github.com/grodansparadis/vscpl2drv-template) | Template for writing new level II drivers (Linux, Windows, macOS). | 2026-11 |
| [vscpl2drv-ikea-dirigera](https://github.com/grodansparadis/vscpl2drv-ikea-dirigera) | Connects the daemon to Matter/Zigbee devices on an IKEA Dirigera hub via its REST API. | 2026-11 |
| [vscpl2drv-websocksrv](https://github.com/grodansparadis/vscpl2drv-websocksrv) | Websocket server driver for VSCP. | 2026-11 |
| [vscpl2drv-multicast](https://github.com/grodansparadis/vscpl2drv-multicast) | Sends and receives VSCP events over IPv4 multicast. | 2026-10 |
| [vscpl2drv-udp](https://github.com/grodansparadis/vscpl2drv-udp) | Sends and receives VSCP events over UDP. | 2026-10 |
| [vscpl2drv-socketcan](https://github.com/grodansparadis/vscpl2drv-socketcan) | Level II driver for Linux SocketCAN. | 2026-10 |
| [vscpl2drv-mqtt](https://github.com/grodansparadis/vscpl2drv-mqtt) | MQTT driver for VSCP (not yet ported to v14). | 2026-10 |
| [vscpl2drv-energy-p1](https://github.com/grodansparadis/vscpl2drv-energy-p1) | Reads P1 smart-meter data and emits VSCP measurement events. | 2026-09 |
| [vscpl2drv-tcpipsrv](https://github.com/grodansparadis/vscpl2drv-tcpipsrv) | tcp/ip link protocol server driver. | 2026-08 |
| [vscpl2drv-logger](https://github.com/grodansparadis/vscpl2drv-logger) | Logs VSCP events to file. | 2026-08 |
| [vscpl2drv-tcpiplink](https://github.com/grodansparadis/vscpl2drv-tcpiplink) | Bridges events to/from a remote VSCP tcp/ip link host. | 2025-11 |
| [vscpl2drv-websrv](https://github.com/grodansparadis/vscpl2drv-websrv) | Web, websocket (ws1/ws2) and REST server driver. | 2025-10 |
| [vscpl2drv-rpimax6675](https://github.com/grodansparadis/vscpl2drv-rpimax6675) | Reads a MAX6675 thermocouple converter on Raspberry Pi. | 2022-10 |
| [vscpl2drv-rpi-bh1750](https://github.com/grodansparadis/vscpl2drv-rpi-bh1750) | BH1750 ambient light sensor driver. | 2021-05 |
| [vscpl2drv-rpi-veml7700](https://github.com/grodansparadis/vscpl2drv-rpi-veml7700) | VEML7700 ambient light sensor driver. | 2021-05 |
| [vscpl2drv-automation](https://github.com/grodansparadis/vscpl2drv-automation) | Sends automation events (sunrise, sunset etc.) from location data. | 2021-04 |
| [vscpl2drv-table](https://github.com/grodansparadis/vscpl2drv-table) | Table functionality carried over from the pre-14.0 daemon. | 2020-12 |
| [vscpl2drv-dm](https://github.com/grodansparadis/vscpl2drv-dm) | Decision matrix carried over from the pre-14.0 daemon. | 2019-11 |
| [vscpl2drv-variable](https://github.com/grodansparadis/vscpl2drv-variable) | Variable handling carried over from the pre-14.0 daemon. | 2019-11 |
| [vscpl2drv-sim](https://github.com/grodansparadis/vscpl2drv-sim) | Simulator driver. | 2019-11 |
| [vscpl2drv-rpilcd](https://github.com/grodansparadis/vscpl2drv-rpilcd) | Drives LCDs on Raspberry Pi. | 2019-11 |
| [vscpl2drv-rpigpio](https://github.com/grodansparadis/vscpl2drv-rpigpio) | Raspberry Pi GPIO driver. | 2019-11 |
| [vscpl2drv-raweth](https://github.com/grodansparadis/vscpl2drv-raweth) | VSCP over raw Ethernet. | 2019-11 |
| [vscpl2drv-parallel](https://github.com/grodansparadis/vscpl2drv-parallel) | Controls a PC parallel port. | 2019-11 |
| [vscpl2drv-lmsensors](https://github.com/grodansparadis/vscpl2drv-lmsensors) | Reads sensors exported as files (lm-sensors). | 2019-11 |
| [vscpl2drv-btdetect](https://github.com/grodansparadis/vscpl2drv-btdetect) | Detects nearby Bluetooth devices. | 2019-11 |
| [vscpl3drv-template](https://github.com/grodansparadis/vscpl3drv-template) | Templates for level III drivers. | 2019-11 |
| [vscpl2drv-wire1](https://github.com/grodansparadis/vscpl2drv-wire1) | 1-Wire driver. | 2019-11 |

## Level I (CANAL) drivers (21)

CAN abstraction layer drivers for adapters and buses. Most were last touched in a January 2020 cleanup.

| Repository | What it is | Last push |
| --- | --- | --- |
| [vscpl1drv-can4vscp](https://github.com/grodansparadis/vscpl1drv-can4vscp) | CANAL driver for CAN4VSCP serial devices. | 2026-09 |
| [vscpl1drv-can232](https://github.com/grodansparadis/vscpl1drv-can232) | CANAL driver for the Lawicel CAN232 adapter. | 2026-09 |
| [vscpl1drv-logger](https://github.com/grodansparadis/vscpl1drv-logger) | Logs level I events to a text file. | 2024-04 |
| [vscpl1drv-socketcan](https://github.com/grodansparadis/vscpl1drv-socketcan) | CANAL driver for Linux SocketCAN. | 2022-01 |
| [vscpl1drv-xap](https://github.com/grodansparadis/vscpl1drv-xap) | Driver for xAP (project ended; unsupported). **Archived.** | 2020-01 |
| [vscpl1drv-zanthic](https://github.com/grodansparadis/vscpl1drv-zanthic) | Driver for the Zanthic CAN adapter. | 2020-01 |
| [vscpl1drv-vectorxl](https://github.com/grodansparadis/vscpl1drv-vectorxl) | Driver for Vector XL CAN. | 2020-01 |
| [vscpl1drv-vector](https://github.com/grodansparadis/vscpl1drv-vector) | Driver for Vector CAN adapters. | 2020-01 |
| [vscpl1drv-usb2can](https://github.com/grodansparadis/vscpl1drv-usb2can) | Driver for USB2CAN. | 2020-01 |
| [vscpl1drv-template](https://github.com/grodansparadis/vscpl1drv-template) | Template level I driver. | 2020-01 |
| [vscpl1drv-tellstick](https://github.com/grodansparadis/vscpl1drv-tellstick) | Driver for Tellstick devices. | 2020-01 |
| [vscpl1drv-syslog](https://github.com/grodansparadis/vscpl1drv-syslog) | Syslog driver. | 2020-01 |
| [vscpl1drv-serial](https://github.com/grodansparadis/vscpl1drv-serial) | VSCP over a serial line. | 2020-01 |
| [vscpl1drv-rs485](https://github.com/grodansparadis/vscpl1drv-rs485) | RS-485 driver. | 2020-01 |
| [vscpl1drv-proximity](https://github.com/grodansparadis/vscpl1drv-proximity) | Proximity driver. | 2020-01 |
| [vscpl1drv-peak](https://github.com/grodansparadis/vscpl1drv-peak) | Driver for PEAK CAN adapters. | 2020-01 |
| [vscpl1drv-lincan](https://github.com/grodansparadis/vscpl1drv-lincan) | Driver for LinCAN. | 2020-01 |
| [vscpl1drv-ixxatvc2](https://github.com/grodansparadis/vscpl1drv-ixxatvc2) | Driver for IXXAT VCI2 CAN. | 2020-01 |
| [vscpl1drv-ccs](https://github.com/grodansparadis/vscpl1drv-ccs) | Driver for CCS CAN adapters. | 2020-01 |
| [vscpl1drv-apox](https://github.com/grodansparadis/vscpl1drv-apox) | Driver for the APOX CAN adapter (Windows). | 2020-01 |
| [vscpl1drv-lirc](https://github.com/grodansparadis/vscpl1drv-lirc) | Driver for LIRC infrared. | 2020-01 |

## Python libraries and scripts (18)

PyPI packages plus small scripts that push sensor readings into VSCP.

| Repository | What it is | Last push |
| --- | --- | --- |
| [vscp-python-sensor-bme280](https://github.com/grodansparadis/vscp-python-sensor-bme280) | Publishes BME280 temperature, pressure, humidity and dew point as VSCP events. | 2026-10 |
| [vscp-python-tools](https://github.com/grodansparadis/vscp-python-tools) | Miscellaneous Python tools for VSCP. | 2026-09 |
| [pyvscphelper](https://github.com/grodansparadis/pyvscphelper) | Python wrapper for vscp-helper-lib (PyPI). | 2026-09 |
| [pyvscp](https://github.com/grodansparadis/pyvscp) | Official VSCP Python library (PyPI). | 2026-09 |
| [pyvscpclasses](https://github.com/grodansparadis/pyvscpclasses) | VSCP class constants for Python. | 2026-09 |
| [pyvscptypes](https://github.com/grodansparadis/pyvscptypes) | VSCP type constants for Python. | 2026-09 |
| [vscp-python-sensor-bme680](https://github.com/grodansparadis/vscp-python-sensor-bme680) | Publishes BME680 sensor data to MQTT as VSCP events. | 2026-09 |
| [vscp-python-p1-power-meter](https://github.com/grodansparadis/vscp-python-p1-power-meter) | Publishes P1 smart-meter data (tested on Sagemcom T211). | 2022-08 |
| [vscp-python-send-bh1750](https://github.com/grodansparadis/vscp-python-send-bh1750) | Sends BH1750 lux readings to a VSCP daemon. | 2021-09 |
| [vscp-python-send-veml6070](https://github.com/grodansparadis/vscp-python-send-veml6070) | Sends VEML6070 UV index to a VSCP daemon. | 2021-09 |
| [vscp-python-send-heartbeat](https://github.com/grodansparadis/vscp-python-send-heartbeat) | Sends VSCP heartbeats over MQTT or tcp/ip. | 2021-09 |
| [vscp-python-send-file-measurement](https://github.com/grodansparadis/vscp-python-send-file-measurement) | Sends a value from stdin as a level II measurement. | 2021-09 |
| [vscp-python-sensorpuck](https://github.com/grodansparadis/vscp-python-sensorpuck) | Reads a Silicon Labs Sensor Puck. | 2021-08 |
| [vscp-python-digitemp](https://github.com/grodansparadis/vscp-python-digitemp) | Forwards digitemp 1-Wire readings over the tcp/ip link. | 2021-06 |
| [pyvscpws2](https://github.com/grodansparadis/pyvscpws2) | Websocket ws2 client for Python. | 2020-10 |
| [pyvscpudp](https://github.com/grodansparadis/pyvscpudp) | VSCP over UDP for Python. | 2020-10 |
| [pyvscpws1](https://github.com/grodansparadis/pyvscpws1) | Websocket ws1 client for Python. | 2020-10 |
| [pyvscpmulticast](https://github.com/grodansparadis/pyvscpmulticast) | VSCP over multicast for Python. | 2020-10 |

## Node.js, Node-RED and Arduino (15)

npm packages, Node-RED nodes and an Arduino client.

| Repository | What it is | Last push |
| --- | --- | --- |
| [node-red-contrib-socketcan](https://github.com/grodansparadis/node-red-contrib-socketcan) | Node-RED SocketCAN nodes (built on node-can). | 2026-10 |
| [node-vscp-type](https://github.com/grodansparadis/node-vscp-type) | VSCP type constants for node.js. | 2026-10 |
| [node-vscp-class](https://github.com/grodansparadis/node-vscp-class) | VSCP class constants for node.js. | 2026-10 |
| [node-vscp-helper](https://github.com/grodansparadis/node-vscp-helper) | node.js wrapper for vscp-helper-lib. | 2026-10 |
| [node-red-contrib-vscp-tcp](https://github.com/grodansparadis/node-red-contrib-vscp-tcp) | Node-RED input/output nodes for the VSCP tcp/ip link. | 2026-10 |
| [node-vscp](https://github.com/grodansparadis/node-vscp) | Utilities and constants for VSCP in node.js. | 2026-10 |
| [node-red-contrib-vscp](https://github.com/grodansparadis/node-red-contrib-vscp) | Node-RED nodes for building VSCP flows. | 2026-08 |
| [node-vscp-tcp](https://github.com/grodansparadis/node-vscp-tcp) | tcp/ip link client for node.js. | 2026-08 |
| [node-red-contrib-canal](https://github.com/grodansparadis/node-red-contrib-canal) | Node-RED node for a CANAL driver. | 2026-01 |
| [node-canal](https://github.com/grodansparadis/node-canal) | CANAL driver interface for node.js. | 2025-05 |
| [node-float-helpers](https://github.com/grodansparadis/node-float-helpers) | Helpers for floats and integers in binary buffers. **Archived.** | 2022-03 |
| [node-vscp-ws2](https://github.com/grodansparadis/node-vscp-ws2) | Websocket ws2 client for node.js. | 2021-08 |
| [node-vscp-ws1](https://github.com/grodansparadis/node-vscp-ws1) | No README; websocket ws1 client for node.js. | 2021-08 |
| [node-vscp-rest](https://github.com/grodansparadis/node-vscp-rest) | No README; REST client for node.js. | 2021-08 |
| [arduino-vscptcpclient](https://github.com/grodansparadis/arduino-vscptcpclient) | VSCP tcp/ip client library for Arduino. | 2020-09 |

## CAN4VSCP hardware modules (26)

Hardware and firmware for modules on the CAN4VSCP bus, named after cities. Most were refreshed on 5 May 2026.

| Repository | What it is | Last push |
| --- | --- | --- |
| [can4vscp-power-injector](https://github.com/grodansparadis/can4vscp-power-injector) | Powers the bus and up to 16 modules. | 2026-08 |
| [can4vscp-frankfurt-eth](https://github.com/grodansparadis/can4vscp-frankfurt-eth) | Frankfurt Eth: tcp/ip and MQTT gateway, with or without PoE. | 2026-07 |
| [can4vscp-trunk-adapter](https://github.com/grodansparadis/can4vscp-trunk-adapter) | Trunk adapter for RJ45/CAT-cable wiring between nodes. | 2026-06 |
| [can4vscp-frankfurt-usb](https://github.com/grodansparadis/can4vscp-frankfurt-usb) | Frankfurt USB: buffered, isolated high-speed USB interface (planned). | 2026-05 |
| [can4vscp-vilnius](https://github.com/grodansparadis/can4vscp-vilnius) | Vilnius: A/D module. | 2026-05 |
| [can4vscp-sanfrancisco-barometer](https://github.com/grodansparadis/can4vscp-sanfrancisco-barometer) | San Francisco barometer module. | 2026-05 |
| [can4vscp-paris](https://github.com/grodansparadis/can4vscp-paris) | Paris: relay module. | 2026-05 |
| [can4vscp-odessa](https://github.com/grodansparadis/can4vscp-odessa) | Odessa module. | 2026-05 |
| [can4vscp-newyork-lcd-display-controller](https://github.com/grodansparadis/can4vscp-newyork-lcd-display-controller) | New York LCD display controller. | 2026-05 |
| [can4vscp-kelvin-thermocouple](https://github.com/grodansparadis/can4vscp-kelvin-thermocouple) | Kelvin single-channel thermocouple module. | 2026-05 |
| [can4vscp-kelvin-ntc10k](https://github.com/grodansparadis/can4vscp-kelvin-ntc10k) | Kelvin NTC10K temperature module. | 2026-05 |
| [can4vscp-kelvin-multisensor](https://github.com/grodansparadis/can4vscp-kelvin-multisensor) | Kelvin multisensor module. | 2026-05 |
| [can4vscp-kelvin-9700](https://github.com/grodansparadis/can4vscp-kelvin-9700) | Kelvin module with five MCP9700 temperature channels. | 2026-05 |
| [can4vscp-kelvin-1wire](https://github.com/grodansparadis/can4vscp-kelvin-1wire) | Kelvin 1-Wire temperature module (-55 to +125 °C). | 2026-05 |
| [can4vscp_humidity](https://github.com/grodansparadis/can4vscp_humidity) | No README; humidity module. | 2026-05 |
| [can4vscp-frankfurt-wifi](https://github.com/grodansparadis/can4vscp-frankfurt-wifi) | Frankfurt WiFi gateway. | 2026-05 |
| [can4vscp-frankfurt-rs485](https://github.com/grodansparadis/can4vscp-frankfurt-rs485) | Frankfurt RS-485 bridge. | 2026-05 |
| [can4vscp-frankfurt-rs232](https://github.com/grodansparadis/can4vscp-frankfurt-rs232) | Frankfurt RS-232 interface, bus powered. | 2026-05 |
| [can4vscp-frankfurt-lora](https://github.com/grodansparadis/can4vscp-frankfurt-lora) | Frankfurt LoRa gateway. | 2026-05 |
| [can4vscp-frankfurt-gprs](https://github.com/grodansparadis/can4vscp-frankfurt-gprs) | Frankfurt GPRS node. | 2026-05 |
| [can4vscp-frankfurt-bridge](https://github.com/grodansparadis/can4vscp-frankfurt-bridge) | Isolating bridge between two bus segments. | 2026-05 |
| [can4vscp-frankfurt-ble](https://github.com/grodansparadis/can4vscp-frankfurt-ble) | Frankfurt Bluetooth LE bridge. | 2026-05 |
| [can4vscp-beijing](https://github.com/grodansparadis/can4vscp-beijing) | Beijing: 10-channel general I/O module. | 2026-05 |
| [can4vscp-accra](https://github.com/grodansparadis/can4vscp-accra) | Accra: general counter module. | 2026-05 |
| [can4vscp-los-temp-control](https://github.com/grodansparadis/can4vscp-los-temp-control) | Two 1-Wire temperature channels, humidity and a 5 V relay output. | 2025-09 |
| [can4vscp_kelvin_smt160](https://github.com/grodansparadis/can4vscp_kelvin_smt160) | Kelvin module with five SMT-160 temperature channels. **Archived.** | 2015-12 |

## DIN, P1 and other boards (25)

Newer DIN-rail controllers, gateways and smart-meter (P1/HAN) interfaces; several are prototypes.

| Repository | What it is | Last push |
| --- | --- | --- |
| [vscp-smart-han-port-interface](https://github.com/grodansparadis/vscp-smart-han-port-interface) | BLE/WiFi P1 data logger (prototype). | 2026-08 |
| [vscp-stm32-wiznet-eth](https://github.com/grodansparadis/vscp-stm32-wiznet-eth) | Ethernet board with STM32C5 and WIZnet W5500. | 2026-07 |
| [vscp-din-wireless-esp32-can-z102](https://github.com/grodansparadis/vscp-din-wireless-esp32-can-z102) | Secure WiFi gateway for a remote CAN4VSCP bus. | 2026-06 |
| [vscp-din-m1-base-board-z105](https://github.com/grodansparadis/vscp-din-m1-base-board-z105) | Base board for all DIN M1 modules (prototype). | 2026-04 |
| [vscp-din-wireless-esp32-can-z105](https://github.com/grodansparadis/vscp-din-wireless-esp32-can-z105) | ESP32 CAN FD controller with BT and WiFi (prototype). | 2026-04 |
| [vscp-din-rp2350a-can-z102](https://github.com/grodansparadis/vscp-din-rp2350a-can-z102) | RP2350A WiFi/BLE gateway for CAN4VSCP (work in progress). | 2025-09 |
| [vscp-din-m1-wireless-p1](https://github.com/grodansparadis/vscp-din-m1-wireless-p1) | DIN M1 application board that reads P1 meter data. | 2025-09 |
| [vscp-din-wireless-esp32-flat](https://github.com/grodansparadis/vscp-din-wireless-esp32-flat) | ESP32 controller in a flat DIN package (prototype). | 2025-09 |
| [vscp-din-wireless-esp32-accra](https://github.com/grodansparadis/vscp-din-wireless-esp32-accra) | No README; ESP32 counterpart of Accra. | 2025-09 |
| [vscp-din-wireless-esp32-beijing](https://github.com/grodansparadis/vscp-din-wireless-esp32-beijing) | No README; ESP32 counterpart of Beijing. | 2025-09 |
| [vscp-din-wireless-esp32-vilnius](https://github.com/grodansparadis/vscp-din-wireless-esp32-vilnius) | No README; ESP32 counterpart of Vilnius. | 2023-04 |
| [vscp-din-wireless-esp32-odessa](https://github.com/grodansparadis/vscp-din-wireless-esp32-odessa) | No README; ESP32 counterpart of Odessa. | 2023-04 |
| [vscp-din-wireless-esp32-newyork](https://github.com/grodansparadis/vscp-din-wireless-esp32-newyork) | No README; ESP32 counterpart of New York. | 2023-04 |
| [vscp-din-wireless-esp32-kelvin-ntc10k](https://github.com/grodansparadis/vscp-din-wireless-esp32-kelvin-ntc10k) | No README; ESP32 counterpart of Kelvin NTC10K. | 2023-04 |
| [vscp-din-wireless-esp32-paris](https://github.com/grodansparadis/vscp-din-wireless-esp32-paris) | No README; ESP32 counterpart of Paris. | 2023-04 |
| [vscp-p1-pulse-interface](https://github.com/grodansparadis/vscp-p1-pulse-interface) | HAN P1 to serial TTL interface (sold at frogshop.se). | 2023-01 |
| [vscp-p1-interface-rs232](https://github.com/grodansparadis/vscp-p1-interface-rs232) | P1 HAN port to RS-232 interface. | 2023-01 |
| [vscp-p1-interface-usb](https://github.com/grodansparadis/vscp-p1-interface-usb) | P1 HAN port to USB interface. | 2022-09 |
| [vscp-power-module-2575](https://github.com/grodansparadis/vscp-power-module-2575) | Power module (LM2575). | 2022-09 |
| [vscp-din-eth-rp2040-can](https://github.com/grodansparadis/vscp-din-eth-rp2040-can) | RP2040 Ethernet-to-CAN design. | 2022-06 |
| [vscp-din-wireless-esp8266-flat](https://github.com/grodansparadis/vscp-din-wireless-esp8266-flat) | Flat ESP8266 DIN controller for P1 meters. | 2022-02 |
| [vscp-din-wireless-esp32](https://github.com/grodansparadis/vscp-din-wireless-esp32) | ESP32 WiFi/BT controller, single-module DIN (prototype). | 2022-02 |
| [vscp-din-wireless-esp8266](https://github.com/grodansparadis/vscp-din-wireless-esp8266) | ESP8266 DIN controller reporting P1 meter readings. | 2022-02 |
| [vscp-din-m1-top](https://github.com/grodansparadis/vscp-din-m1-top) | Earlier DIN M1 base board (prototype). | 2021-08 |
| [vscp-blitz](https://github.com/grodansparadis/vscp-blitz) | Blitz: WiFi + Ethernet + CAN board. | 2018-05 |

## Demos and examples (29)

Reference applications and protocol examples across Pico, STM32, ESP8266/ESP32 and Raspberry Pi.

| Repository | What it is | Last push |
| --- | --- | --- |
| [vscp-demo-link-stm32f401-wiz-ip20-blinky](https://github.com/grodansparadis/vscp-demo-link-stm32f401-wiz-ip20-blinky) | Blinky over VSCP link on STM32F401 + WIZnet IP20. | 2026-08 |
| [vscp-demo-link-stm32f103-wiz-ip20-blinky](https://github.com/grodansparadis/vscp-demo-link-stm32f103-wiz-ip20-blinky) | Blinky over VSCP link on STM32F103 Blue Pill + WIZnet IP20. | 2026-06 |
| [vscp-demo-pico-wiznet-tcpiplinksrv](https://github.com/grodansparadis/vscp-demo-pico-wiznet-tcpiplinksrv) | VSCP link server on a Pico with W5100S HAT. | 2026-05 |
| [vscp-demo-wiznet-w55rp20-nfc](https://github.com/grodansparadis/vscp-demo-wiznet-w55rp20-nfc) | Distributed NFC lock using VSCP link and multicast on W55RP20. | 2026-05 |
| [vscp-demo-pico-fridge-ctrl](https://github.com/grodansparadis/vscp-demo-pico-fridge-ctrl) | Fridge temperature control over MQTT on W55RP20-EVB-Pico. | 2026-02 |
| [vscp-udp](https://github.com/grodansparadis/vscp-udp) | UDP send examples for Linux and Windows, optionally encrypted. | 2026-02 |
| [vscp-multicast](https://github.com/grodansparadis/vscp-multicast) | Multicast send examples for Linux and Windows, optionally encrypted. | 2025-06 |
| [vscp-ble-esp32](https://github.com/grodansparadis/vscp-ble-esp32) | ESP32 BLE example code. | 2025-04 |
| [vscp-espnow](https://github.com/grodansparadis/vscp-espnow) | VSCP over ESP-NOW wireless; continues vscp-droplet. | 2025-03 |
| [dumb-fridge-controler](https://github.com/grodansparadis/dumb-fridge-controler) | Standalone RP2040 fridge controller with NTC and I2C LCD. | 2024-11 |
| [vscp-demo-blink](https://github.com/grodansparadis/vscp-demo-blink) | Minimal blink demo on several platforms; starting point for new code. | 2024-10 |
| [vscp-demo-mqtt-web](https://github.com/grodansparadis/vscp-demo-mqtt-web) | VSCP over MQTT websockets in the browser (work in progress). | 2023-10 |
| [vscp-droplet](https://github.com/grodansparadis/vscp-droplet) | Self-forming wireless network; superseded by vscp-espnow. | 2023-04 |
| [vscp-demo-svg-dynamic](https://github.com/grodansparadis/vscp-demo-svg-dynamic) | UI with dynamically updated SVG. | 2022-10 |
| [vscp-demo-gas-sensor](https://github.com/grodansparadis/vscp-demo-gas-sensor) | CO monitor with an MQ-7 sensor (work in progress). | 2022-10 |
| [esp32_start_project](https://github.com/grodansparadis/esp32_start_project) | VS Code + ESP-IDF starter project with JTAG debugging on Linux. | 2022-09 |
| [vscp-demo-viking-9900](https://github.com/grodansparadis/vscp-demo-viking-9900) | PLC interface for a Viking 9900 pellet burner (prototype). | 2022-02 |
| [vscp-demo-influxdb-web](https://github.com/grodansparadis/vscp-demo-influxdb-web) | InfluxDB demo pages for the VSCP demo server. | 2022-01 |
| [vscp-demo-proximity-sensor](https://github.com/grodansparadis/vscp-demo-proximity-sensor) | Battery ESP8266 proximity sensor reporting to VSCP or MQTT. | 2021-05 |
| [vscp-demo-proximity-esp8266-nonos](https://github.com/grodansparadis/vscp-demo-proximity-esp8266-nonos) | ESP8266 NONOS variant (under construction). | 2021-05 |
| [vscp-demo-proximity-esp8266-rtos](https://github.com/grodansparadis/vscp-demo-proximity-esp8266-rtos) | ESP8266 RTOS variant (under construction). | 2021-05 |
| [tasmota2vscp](https://github.com/grodansparadis/tasmota2vscp) | Translates Tasmota to and from VSCP. | 2020-10 |
| [vscp-demo-mailbox-control](https://github.com/grodansparadis/vscp-demo-mailbox-control) | Battery ESP8266 sensor that reports when the mailbox lid opens. | 2020-10 |
| [vscp-demo-power-meter-interface](https://github.com/grodansparadis/vscp-demo-power-meter-interface) | House power monitoring with Accra, Vilnius and Kelvin modules. | 2020-10 |
| [vscp-demo-lcd-hd44780-rpi](https://github.com/grodansparadis/vscp-demo-lcd-hd44780-rpi) | HD44780 LCD library in Python and C for Raspberry Pi. | 2020-05 |
| [vscp-demo-furnace-heating-system](https://github.com/grodansparadis/vscp-demo-furnace-heating-system) | Heating system demo with Accra and Vilnius. | 2018-11 |
| [vscp-demo-water-heating-system](https://github.com/grodansparadis/vscp-demo-water-heating-system) | Water heating demo with Accra, Paris and Kelvin NTC10K. | 2018-11 |
| [vscp-demo-fridge-control](https://github.com/grodansparadis/vscp-demo-fridge-control) | Fridge/refrigerator control demo. | 2018-09 |
| [vscp-demo-weatherstation](https://github.com/grodansparadis/vscp-demo-weatherstation) | Weather station demo with Accra and Vilnius. | 2018-08 |

## Forks and non-VSCP (14)

Forks of third-party projects plus repositories unrelated to VSCP.

| Repository | What it is | Last push |
| --- | --- | --- |
| [Halsingetoppen](https://github.com/grodansparadis/Halsingetoppen) | Chart of top artists from Hälsingland (Swedish). | 2026-10 |
| [grodansparadis](https://github.com/grodansparadis/grodansparadis) | GitHub profile README. | 2026-10 |
| [can4vscp-frdm-rtos](https://github.com/grodansparadis/can4vscp-frdm-rtos) | NXP FRDM board firmware, RTOS variant (work in progress). | 2025-09 |
| [can4vscp-frdm-bm](https://github.com/grodansparadis/can4vscp-frdm-bm) | NXP FRDM board firmware, bare-metal variant. | 2025-09 |
| [RadioLib](https://github.com/grodansparadis/RadioLib) | Universal wireless communication library for embedded devices. | 2025-09 |
| [vscp-demo-max6675-rpi](https://github.com/grodansparadis/vscp-demo-max6675-rpi) | MAX6675 thermocouple demo for Raspberry Pi. | 2024-10 |
| [esp-now](https://github.com/grodansparadis/esp-now) | Espressif ESP-NOW component. | 2023-05 |
| [nordpool-mqtt](https://github.com/grodansparadis/nordpool-mqtt) | Publishes Nord Pool power prices to MQTT. | 2022-09 |
| [fastpbkdf2](https://github.com/grodansparadis/fastpbkdf2) | Fast PBKDF2-HMAC implementation in C. | 2022-02 |
| [MQTT2CAN](https://github.com/grodansparadis/MQTT2CAN) | MQTT gateway for SocketCAN. | 2020-08 |
| [node_red_weather_observations_website](https://github.com/grodansparadis/node_red_weather_observations_website) | Node-RED flow posting MQTT weather data to the Met Office WOW site. | 2018-12 |
| [VSCP-Helper-for-Python](https://github.com/grodansparadis/VSCP-Helper-for-Python) | Early Python wrapper for the helper library. | 2015-08 |
| [vscp-arduino](https://github.com/grodansparadis/vscp-arduino) | VSCP Arduino library. | 2015-08 |
| [usb2can](https://github.com/grodansparadis/usb2can) | Linux driver for the 8devices USB2CAN. | 2014-12 |

Source: the GitHub repositories under [grodansparadis](https://github.com/grodansparadis), with descriptions taken from each README on 2026-11-11.

[filename](./bottom_copyright.md ':include')
