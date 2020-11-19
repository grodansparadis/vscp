# Sample vscpd configuration file

 
##### Windows

`<code="xml">`
`<?xml version = "1.0" encoding = "UTF-8" ?>`

`<!-- Version 0.5.0	   2014-09-19 		                      -->`
`<!-- Example configuration file 		                      -->`
`<!-- Ake Hedman, akhe@grodansparadis.com                      -->`
`<!-- http://www.grodansparadis.com	                          -->`
`<!-- Documentation for this configuration file is available   -->`
`<!-- in the specification section 14.4 which can be found     -->`
`<!-- here http://vscp.org/docs.php                            -->`

`<vscpconfig>`

    `<general>`
    
        `<!-- Set the detail of logging you want -->`
        `<!-- Documented in specification 14.4.1 -->`

        `<loglevel>`debug`</loglevel>`
        
        `<!-- General logging will be done to syslog on Unix but on       -->`
        `<!-- windows this path must be set to get logging saved into a   -->`
        `<!-- file. The debug level will determine how much information   -->`
        `<!-- you will get into this file.                                -->`
        
        `<generallogfile enable="true">`c:/tmp/vscp_log_general.txt`</generallogfile>`
        
        `<!-- Security logging will be done to this path must be set to    -->`
        `<!-- get security logging saved into a file.                      -->`
        
        `<securitylogfile enable="true">`c:/tmp/vscp_log_security.txt`</securitylogfile>`
        
        `<!-- Access logging will be done to this path must be set to        -->`
        `<!-- get access logging for tcp/ip udp webserver websocket and rest -->`
        `<!-- saved into a file.                                             -->`
        
        `<accesslogfile enable="true">`c:/tmp/vscp_log_access.txt`</accesslogfile>`

        `<!-- It is possible to enable/disable the TCP/IP interface        -->`
        `<!-- You can open TCP/IP interfaces on several ports or on        -->`
        `<!-- on a specific port or on every interface of the computer     -->`
        `<!-- Default 9598 will listen on all interfaces while             -->`
		<!-- "127.0.0.1:9598" only will listen on the specified interface.-->
		<!-- To specify several interfaces just enter them with a space   -->
        `<!-- between them.                                                -->`
        `<!-- Documented in specification 14.4.1                           -->`

        `<tcpip enable="true" interface="9598"/>`
		
		<!-- It is possible to enable/disable the UDP interface           -->
        `<!-- You can open UDP interfaces on several ports or on           -->`
        `<!-- on a specific port or on every interface of the computer     -->`
        `<!-- Default "UDP://:9598" will listen on all interfaces while    -->`
		<!-- "UDP://127.0.0.1:9598" only will listen on the specified     -->
		<!-- interface. To specify several interfaces just enter them     -->
        `<!-- with a space between them. If interface is not preceded with -->`
		<!-- "UDP://" it will be added.                                   -->
		<!-- Documented in specification 14.4.1                           -->
		
		<udp enable="true" interface="udp://9598"/>


        `<!-- Set to true to enable VSCP high end functionality -->`
        `<!-- Documented in specification 14.4.1                -->`

        `<vscp enable="true"/>`

        `<!-- Set to true to enable decision matrix                         -->`
        `<!-- Set to false to disable.                                       -->`
        `<!-- Set path to the decision matrix file location                 -->`
        `<!-- Documented in specification 14.4.1                            -->`
        `<!-- Decision matrix logging: loglevel="minor|normal|extra|debug"  -->`
        `<!-- enableogging="true:false" to enable/disable dm logging        -->`
        `<!-- logpath is ptah to logfile.                                   -->`
        
        <dm enable="true" 
                path="/etc/vscp/dm.xml" 
                enablelogging="true" 
                loglevel="debug" 
                logpath="c:/tmp/vscp_log_dm.txt" />      

        `<!-- Set to true to enable daemon variables         -->`
        `<!-- Set to false to disable.                       -->`
        `<!-- Path to variables storage.                    -->`
        `<!-- Documented in specification 14.4.1            -->`

        `<variables enable="true" path="/tmp/variables.xml"/>`

        `<!-- Set GUID for daemon                               -->`
        `<!-- Read more about GUID's in Specification section 4 -->`
        `<!-- Apply for your own GUID series at guid@vscp.org   -->`
        `<!-- Documented in specification 14.4.1                -->`
        `<guid>`
            FF:FF:FF:FF:FF:FF:FF:FF:FF:FF:FF:F5:00:00:00:00
        `</guid>`

        `<!-- This is the default buffer size (number of events) for all  -->`
        `<!-- clients in the system. Everything from a driver to a TCP/IP -->`
        `<!-- user is regarded as a client. Default is 1024.              -->`
        `<!-- Documented in specification 14.4.1                          -->`
        `<clientbuffersize>`1024`</clientbuffersize>` 

        
        `<!-- Enable disable the webserver interface.          -->`
        `<!-- Set the port to use for the webserver interface  -->`
        `<!-- default is 8080.                                 -->`
        `<!-- Documented in specification 14.4.1               -->`
        `<webserver enable="true" port="8080">`
        
            `<!-- Path to server web root file system              -->`
            `<!-- Server should have read access to this location  -->`
            `<!-- and it is used both fopr websockets and for      -->`
            `<!-- traditional web-pages.                           -->`
            `<!-- Documented in specification 14.4.1               -->`
            `<webrootpath>`C:/Users/grodansparadis/development/vscp_html5`</webrootpath>`
            
            `<!-- authDomain is the domain name that will be used  -->`
            `<!-- when the internal server talks to clients doing  -->`
            `<!-- authentications and other things.                -->`
            `<authdomain>`mydomain.com`</authdomain>`
		
            `<!-- Path to ssl certificate                          -->`
            `<!-- Only valid for the SSL version                   -->`
            `<!-- Documented in specification 14.4.1               -->`
            `<pathcert>`C:/ProgramData/vscp/certs/ssl_cert.pem`</pathcert>`

            `<!-- Path to ssl key                                  -->`
            `<!-- Only valid for the SSL version                   -->`
            `<!-- Documented in specification 14.4.1               -->`
            `<pathkey>`/etc/vscp/sslcert`</pathkey>`
            
            `<!-- Enable disable the websocket interface.                    -->`
            `<!-- Id auth is set to false not authentication will be done    -->`
            `<!-- Documented in specification 14.4.1                         -->`
        
            `<websockets enable="true" auth="true" />`
        
            `<!-- Path to file holding mime types                  -->`
            `<!-- Documented in specification 14.4.1               -->`
            `<pathtomimetypes>`/etc/vscp/mimetypes.xml`</pathtomimetypes>`
        
        `</webserver>`    
        
    `</general>`

    
    `<!-- Holds information about one (at least) or more users      -->` 
    `<!-- Use mkpassword to generate a new password                 -->`
    `<!-- Privilege is admin/user                                   -->`
    `<!-- Same information is used for accessing the daemon         -->`
    `<!-- through the TCP/IP interface as through the web-interface -->`
    `<!-- Documented in specification 14.4.2                        -->`
    `<remoteuser>` 
        `<user>`
            `<name>`admin`</name>`
            `<password>`d50c3180375c27927c22e42a379c3f67`</password>`
            `<privilege>`admin`</privilege>`
            `<allowfrom>`127.0.0.1,192.168.1.`</allowfrom>`
        `</user>`	
        `<user>`
            `<name>`user`</name>`
            `<password>`ee11cbb19052e40b07aac0ca060c23ee`</password>`
            `<privilege>`admin`</privilege>`
            `<allowfrom>`127.0.0.1,192.168.1.`</allowfrom>`
        `</user>`	
    `</remoteuser>`

    `<!-- Here are settings that affect the internal VSCP functionality -->`
    `<!-- of the server.                                                -->`
    `<!-- Documented in specification 14.4.3                            -->` 
    `<automation>`
        `<zone>`1`</zone>`
        `<sub-zone>`2`</sub-zone>`
        `<longitude>`61.74418333333333`</longitude>`
        `<latitude>`15.160416666666666`</latitude>`
        `<time enable="true" interval="60" />`
        `<sunrise enable="true" />`
        `<sunset enable="true" />`
        `<heartbeat enable="true" interval="30" />`
    `</automation>`
	
    `<!-- Holds information about interfaces on the                  -->`
    `<!-- machine the daemon is run on. Set ip-address, mac-address  -->`
    `<!-- and GUID for your interface here.                          -->`
    `<!-- !!!! Normally no need to enter this information.           -->`
    `<!-- Documented in specification 14.4.6                         -->`
    `<interfaces>`
        `<interface>` 
            `<ipaddress>`192.168.168.10`</ipaddress>`
            `<macaddress>`00:16:D4:5D:68:F6`</macaddress>`
            `<guid>`01:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00`</guid>`
        `</interface>` 
    `</interfaces>`
	
    `<!-- List level I/CANAL drivers here                            -->`
    `<!-- Documented in specification 14.4.4                         -->`

    `<leve1driver enable="true" >` 

        `<!-- The canallog driver is documented in 27.20.14 -->`
        `<driver enable="false">`
            `<name>`logger`</name>`
            `<config>`/tmp/canallog.txt`</config>`
            `<path>`/ust/local/lib/canallogger.so`</path>`
            `<flags>`1`</flags>`
            `<guid>`00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00`</guid>`
        `</driver>`

        `<!-- The can232 driver is documented in 27.20.3 -->`
        `<driver enable="false" >`
            `<name>`can232`</name>`
            `<config>`/dev/ttyS0;19200;0;0;125`</config>`
            `<path>`/usr/local/lib/can232drv.so`</path>`
            `<guid>`00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00`</guid>`
            `<flags>`0`</flags>`
        `</driver>`

        `<!-- The xap driver is documented in 27.20.17 -->`
        `<driver enable="false" >`
            `<name>`xap`</name>`
            `<config>`9598;3639`</config>`
            `<path>`/usr/local/lib/xapdrv.so`</path>`
            `<flags>`0`</flags>`
            `<guid>`00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00`</guid>`
        `</driver>`

    `</leve1driver>`

	<!-- List level II VSCP drivers here                            -->
    `<!-- Documented in specification 14.4.5                         -->`

    `<vscpdriver enable="true">`	 	

        `<!-- Level II Logger - driver is documented in 30.1 -->`
        `<driver enable="true" >`
            `<name>`VSCPLoggerDriver1`</name>`	
            `<path>`/ust/local/lib/vscpl2_loggerdrv.so`</path>`
             `<config>`/tmp/vscp_level2.log`</config>` 		
             `<guid>`00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00`</guid>`
        `</driver>`
 	
        `<!-- Level II lmsensors - driver is documented in 30.3 -->`
        `<driver enable="false" >`
            `<name>`lmsensors`</name>`
            `<path>`/usr/local/lib/vscp2drv_lmsensors.so`</path>`
            `<config>`2`</config>`
            `<guid>`00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00`</guid>`
        `</driver>`

        `<!-- Level II socketcan - driver is documented in 30.4 -->`
	    <driver enable="false" >
            `<name>`VSCPSocketCANDriver 1`</name>`
            `<path>`/usr/local/lib/linux/vscp2drv_socketcan.so`</path>`
            `<config>`can0`</config>`
            `<guid>`00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00`</guid>`
        `</driver>`

        `<!-- Level II mqtt - driver is documented in 30.6 -->`
        `<driver enable="false" >`
            `<name>`VSCPMQTTDriver1`</name>`
            `<path>`/usr/local/lib/vscp2drv_mqtt.so`</path>`
            `<config>`subscribe;vscp;localhost;1883;;60`</config>`
            `<guid>`08:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00`</guid>`
        `</driver>`	

        `<!-- Level II tcp/ip link - driver is documented in 30.5 -->`
        `<driver enable="false" >`                 
            `<name>`tcpiplink1`</name>`
            `<path>`/usr/local/lib/vscp2drv_tcpiplink.so`</path>`
            `<config>`192.168.1.2;9598;admin;secret`</config>`                 
            `<guid>`00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00`</guid>`
        `</driver>`

        `<!-- Level II raw ethernet - driver is documented in 30.2 -->`
        `<driver enable="false">` 		   	 			
            `<name>`rawethernet1`</name>` 		   	 		
            `<config>`eth0`</config>`
            `<path>`/usr/local/lib/vscp2drv_raweth.so`</path>`
        `</driver>`
 

    `</vscpdriver>`	

`</vscpconfig>`

```

\\ 
----
Copyright (c) 2000-2014 [Åke Hedman](mailto/akhe@grodansparadis.com), [Paradise of the Frog / Grodans Paradis AB](http://www.grodansparadis.com)
