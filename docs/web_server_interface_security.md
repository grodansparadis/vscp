# VSCP Daemon web server security 

The webserver is protected by a user/password pair. The username is sent a digest over the net but the password is a hash over *"username:authdomain|raltext-password"*.([authdomain is described here](./configuring_the_vscp_daemon#configuration))

I addition to the username/password which also groups users in security levels it is possible to have table where the hosts allowed to connect to the system is stored.

In addition to this SSL can be enabled on the interface.

Put together this makes the VSCP Daemon one of the safest systems to use for remote maintenance of IoT/m2m systems.

[filename](./bottom_copyright.md ':include')
