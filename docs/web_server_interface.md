# VSCP Daemon Webserver interface 

The webserver interface of the VSCP daemon is a full webserver that can share pages to users as any other webserver. In addition it has an administration interface on a reserver URI  

    /vscp

here several common administration tasks can be performed such as handling variables, handling the decision matrix, handle interfaces, view logfiles etc. The web interface can also be extended with cgi-scripts.

Default port for the webserver is 

    8843  - ssl.
    8888  - redirect to ssl.
    8884  - open port.

but other ports can be configured in the [configuration file](./configuring_the_vscp_daemon.md#the_general_section).  


[filename](./bottom_copyright.md ':include')
