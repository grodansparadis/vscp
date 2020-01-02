% VSCPD(8) VSCP Daemon
% Åke Hedman, Grodans Paradis AB
% December 18, 2019

# NAME

vscpd - The VSCP Daemon

# SYNOPSIS

vscpd [*options*]

# DESCRIPTION

The VSCP daemon is an open source daemon/server program that is part of VSCP & Friends and is built and maintained by Åke Hedman, [Grodans Paradis AB](https://www.grodansparadis.com). The server act as a hub and collects information from different sources and let clients collect this information over a TCP/IP, websocket, REST interface etc and makes it easy to present or work on this date in real time. The daemon also has an internal scheduler that makes it possible to control different scenarios such as simple tasks like to automatically turn on a group of lamps at a specific time of the day or do complex industrial control tasks.

On Unix systems the daemon is a standard server application that is started in the background. It is possible also to compile it to start as a standard application if that way of running it is preferred.

The daemon uses simple to construct, yet very capable, drivers to talk to the real world. This means that a driver abstract real world things to look to the system as any other VSCP enabled device even if it itself has no knowledge what so ever about VSCP

# OPTIONS

-h
: Give help about commands.

-s
: Standalone (don't run as a daemon). Terminate with ctrl+c.

-r root-path
: Specify VSCP root folder. Default is /srv/vscp.

-c path-to-vscpd-config-file
: Tell where the program should look for the configuration file. Default is to look for this file in /etc/vscp/vscpd.conf.

-k XX:XX:XX:XX:XX:XX:XX:XX:XX:XX:XX:XX:XX:XX:XX:XX:XX....
: Set the 32 byte encryption key string in hex format.

-d debug-level
: Debug level (0-2). 0=Default. Higher value is more output.

-g
: Print MIT license.

# SEE ALSO

`uvscpd` (8)
`vscpworks` (1).
`vscpcmd` (1).
`vscp-makepassword` (1).
`vscphelperlib` (1).
`vscpdrivers` (7)

The VSCP project homepage is here <https://www.vscp.org>.

The [manual](https://grodansparadis.gitbooks.io/the-vscp-daemon) for vscpd contains full documentation. Other documentation can be found here <https://grodansparadis.gitbooks.io>.

The vscpd source code may be downloaded from <https://github.com/grodansparadis/vscp>. Source code for other system components of VSCP & Friends are here <https://github.com/grodansparadis>

# COPYRIGHT
Copyright 2000-2019 Åke Hedman, Grodans Paradis AB - MIT license.
