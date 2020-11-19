# VSCP Daemon websocket server security 

The websocket interface is protected by a user/password pair. The username is sent a digest over the net but the password is a hash over "username:authdomain|raltext-password".((authdomain is described [here]( ./configuring_the_vscp_daemon.md#configuration))

I addition to the username/password which also groups users in security levels it is possible to have table where the hosts allowed to connect to the system is stored.

In addition to this SSL can be enabled on the interface.

Also a privilege based system is used to protect critical functionality. A user need a specific privilege to send an event for example and can be set up to be allowed just to send a limited set of events. Also a filter on incoming events is possible to set up to limit what a user can receive.

There is also a [privilege system](./configuring_the_vscp_daemon.md#remote_user_settings) for the websocket interface just as it is for the TCP/IP interface

 | Command       | Privilege | Description                              | 
 | -------       | :---------: | ----------- | 
 | NOOP          | 0         | No operation                             | 
 | AUTH          | 0         | Authentication                           | 
 | OPEN          | 0         | Open channel                             | 
 | CLOSE         | 0         | Close channel                            | 
 | SETFILTER     | 6         | Set filter                               | 
 | CLRQUEUE      | 1         | Clear input queue                        | 
 | WRITEVAR      | 6         | Write variable                           | 
 | CREATEVAR     | 6         | Create a new variable                    | 
 | READVAR       | 4         | Read variable                            | 
 | RESETVAR      | 6         | Reset variable to default value          | 
 | REMOVEVAR     | 6         | Remove (delete) variable                 | 
 | LENGTHVAR     | 4         | Get length of variable                   | 
 | LASTCHANGEVAR | 4         | Get last change date + time for variable | 
 | LISTVAR       | 4         | List variables                           | 
 | SAVEVAR       | 1         | Save variables                           | 
 | Send event    | 6         | Send event                               | 
 | Read event    | 0         | Read event                               | 

Put together this makes the VSCP Daemon one of the safest systems to use for remote maintenance of IoT/m2m systems.

[filename](./bottom_copyright.md ':include')
