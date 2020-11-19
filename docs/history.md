# Document History

*  **2020-10-27 AKHE** - libmosquitto-dev now required for build.
*  **2020-10-27 AKHE** - Formatted Level II API docs.
*  **2020-10-26 AKHE** - Added section about how to enable Google test
*  **2020-10-15 AKHE** - Added logging section.
*  **2020-10-09 AKHE** - User privilege info for configuration file updated.
*  **2020-10-06 AKHE** - Added UDP server info.
*  **2019-12-16 AKHE** - Started work on 14.0 documentation.
*  **2018-09-24 AKHE** - Added info about new configurations parameters for the tcp/ip level II link driver.
*  **2018-04-04 AKHE** - build added to tcp/ip "version" command output.
*  **2018-04-04 AKHE** - VAR READ response changed from build 13.0.0.4  Added response info for items that missed such info.
*  **2018-02-16 AKHE** - **DM** and **VAR** commands are now part of the [VSCP Link Interface](http://www.vscp.org/docs/vscpd/doku.php?id=vscp_daemon_tcp_ip_protocol_description).
*  **2018-01-04 AKHE** - [debugflags where named logflags](http://www.vscp.org/docs/vscpd/doku.php?id=configuring_the_vscp_daemon#debugflags1). Changed.
*  **2017-12-28 AKHE** - Added configuration for [logflags](http://www.vscp.org/docs/vscpd/doku.php?id=configuring_the_vscp_daemon#logflags1).

*  **2017-12-15 AKHE** - Added [logdays](http://www.vscp.org/docs/vscpd/doku.php?id=configuring_the_vscp_daemon#logdays) to autoclear logs if they dont hold to to old enteries and get to big.

*  **2017-12-13 AKHE** - Added new escapes to print out meaningful data values for diagnostic and debugging.

*  **2017-08-22 AKHE** - [Single quotes replaced with valid JSON double quotes](http://www.vscp.org/docs/vscpd/doku.php?id=javascript_callbacks).

*  **2017-06-22 AKHE** - Version **1.12.26** Multicast and UDP interfaces in place.

*  **2017-06-14 AKHE** - Added *segmentcontrol-event*, *heartbeat-event*, *capabilities-event* to automation in configuration file.

*  **2017-06-13 AKHE** - Version **1.12.24**

*  **2017-06-13 AKHE** - Reading of configuration files has changed. Now the general section is read from the XML file, then the database configuration and after that the full XML configuration.

*  **2017-06-02 AKHE** - The XML configuration file is now read before the database configuration.

*  **2017-02-17 AKHE** - **%isobothms** VSCP decision matrix escape added.

*  **2017-02-17 AKHE** - **%isoboth** VSCP decision matrix escape added.

*  **2017-02-17 AKHE** - JavaScript callbacks and action documented.

*  **2017-02-17 AKHE** - The [%variable](http://www.vscp.org/docs/vscpd/doku.php?id=vscp_daemon_decision_matrix#variable_substitution_for_parameters_escapes) escape has a new format and is changed to %variable:[name] where name is the variable name. Also a new form **%vardecode** is added which do a BASE64 translation of the value (if it is decoded) before the value is written. A new escape %file[path] has been added which replace the escpape with the content of that in the file pointed to by path.

*  **2017-01-25 AKHE** - Some initial documentation for the [JavaScript action](http://www.vscp.org/docs/vscpd/doku.php?id=vscp_daemon_decision_matrix#run_javascript) has been provided.

*  **2016-11-30 AKHE** - The measurement variable type has changed format from *MEASUREMENT|6;true|false;datacoding,data1,…* to *MEASUREMENT|6;true|false;value;unit;sensor-index;zone;subzone* 

*  **2016-11-30 AKHE** - Websocket [AUTH1](http://www.vscp.org/docs/vscpd/doku.php?id=vscp_daemon_websocket_protocol_description#auth) response now return all fields of a user record.

*  **2016-10-25 AKHE** - [AUTH](http://www.vscp.org/docs/vscpd/doku.php?id=vscp_daemon_websocket_protocol_description#auth) response after login on websocket now also give user info back.

*  **2016-10-25 AKHE** - [lstvar](http://www.vscp.org/docs/vscpd/doku.php?id=vscp_daemon_websocket_protocol_description#lstvar) websocket command now also send total which is the total number of variables sent.

*  **2016-08-08 AKHE** - [Translations](http://www.vscp.org/docs/vscpd/doku.php?id=configuring_the_vscp_daemon#level_i_drivers)  in drivers described.

*  **2016-08-08 AKHE** - A new tag has been added in the configuration file `<secret>` which defines admin password and other secret related stuff.

*  **2016-09-02 AKHE** - The possibility to set a mnaula imezone in the configuration file has been removed.                   

*  **2016-08-10 AKHE** - "control" tag deprecated in DM XML file. "measurement" tag added. http://www.vscp.org/docs/vscpd/doku.php?id=vscp_daemon_decision_matrix#the_decision_matrix_file_format

*  **2016-07-14 AKHE** - XML format for variables changed. "`<variables>`" is used instead of "`<persistent>`" as main tag. "`<persistent>`" still works. "persistent", "userid", "groupid", "accessrights" attributes added for variable.

*  **2016-06-02 AKHE** - Loglevels changed to 0-2 (NONE, NORMAL, DEBUG) http://www.vscp.org/docs/vscpd/doku.php?id=configuring_the_vscp_daemon#loglevel

*  **2016-03-20 AKHE** - Added configuration infor for MQTT broker and CoAP server.

*  **2016-03-13 AKHE** - New variable data types added. 

*  **2016-03-11 AKHE** - LUA and SQLite3 install intructions added. 

*  **2016-03-01 AKHE** - Added actions and escapes to menubar.

*  **2016-02-18 AKHE** - **ssi_pattern**, **ip_acl**, **m_dav_document_root**, **per_directory_auth_file** and **global_auth_file**  added as webserver option in config file.

*  **2016-02-10 AKHE** - Sessionid introduced as parameter for MQTT driver. From version. 1.1.0.22 Sodium

*  **2016-02-02 AKHE** - [ArchLinuxArm](http://www.vscp.org/docs/vscpd/doku.php?id=setting_up_the_system_on_an%20embeded%20ArchLinuxArm%20system) install description section added.

*  **2016-02-02 AKHE** - Removed duplicate descriptions of CLASS2.VSCPD events. Now only described in specification.

*  **2016-01-22 AKHE** - Some more examples added to the execute action here http://www.vscp.org/docs/vscpd/doku.php?id=vscp_daemon_decision_matrix#execute_external_program

*  **2016-01-21 AKHE** - %amp, %amp-html, %lt, %lt.html, %gt, %gt.htm escapes added.

*  **2016-01-21 AKHE** - Variable escape now works correctly. 

*  **2016-01-21 AKHE** - All path escapes has changed format from *"path_"* to *"path."* to be consistent with other escapes. http://www.vscp.org/docs/vscpd/doku.php?id=vscp_daemon_decision_matrix#variable_substitution_for_parameters_escapes

*  **2016-01-19 AKHE** - GET/PUT DM action now have method;url;... order instead of url;method;.. http://www.vscp.org/docs/vscpd/doku.php?id=vscp_daemon_decision_matrix#http_get_put_post

*  **2015-12-17 AKHE** - Added note to Macintosh installation.

*  **2015-10-21 AKHE** - "multicast-annouce" added to configuration file.

*  **2015-10-21 AKHE** - 'servername' added to configuration file.

*  **2015-04-28 AKHE** - Added switch 'disableauthentication' to web server configuration that turns of web authentication.

*  **2015-04-28 AKHE** - Added info about how to build wxWidgets 3.0.2 on the Raspberry Pi platform.

*  **2015-04-28 AKHE** - Stated Document history.


[filename](./bottom_copyright.md ':include')
