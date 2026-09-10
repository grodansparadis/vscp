![Very Simple Control Protocol](./images/logo_100.png "Very Simple Control Protocol") 

# The VSCP common code

**Describe software version**:${/var/document-version}      
**Doc update:** ${/var/creation-time}       
[history](./history.md)

Author Åke Hedman and [the VSCP project](https://www.vscp.org), [info@vscp.org](info@vscp.org)  

The documentation for the VSCP MQTT daemon is available [here](https://github.com/grodansparadis/vscp-mqtt-daemon) and of the tcp/ip daemon [here](https://github.com/grodansparadis/vscp-tcpip-daemon). 

# Abstract

The VSCP common code is code that is used by different parts of the VSCP system and have origin back into 2000 when the VSCP project started. It is code that is used by both the core and the different modules. The common code is also used by the different tools that are used to develop and maintain the VSCP system. Much of this code is designed to be reusable and modular, providing a consistent foundation for the various components of the VSCP ecosystem.

This repository previously contained the VSCP daemon and other executables. This code has been lifted out to their respective own repositories. Documentation needed a new structure because of this. This will take some time to fully implement.

Documentation for the common code will be added as we go along. The common code is not a separate module, but rather a collection of code that is used by different parts of the system. Often as a subproject for that project within the larger VSCP ecosystem, it provides essential functionality and shared resources that facilitate the development and maintenance of the entire VSCP system.

[filename](./bottom_copyright.md ':include')


