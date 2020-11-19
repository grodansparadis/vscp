# VSCP Daemon Websocket Protocol Description

From version 14.0.0 there is two versions of the VSCP daemon websocket interface. **ws1** which is the original implementation and **ws2** which is a JSON based implementation. The same set of command are available for both.

Furthermore the variable and table handling commands has been removed. Support for this functionality will be provided in higher levels of the VSCP family of functionality. 

You enable both versions by setting the _enable_ attribute to true in the vscpd.conf configuration file.

```xml
<websockets enable="true" />
```
You reach the ws1 interface at 

```url
http[s]://server:port/ws1
```

and the ws2 interface at

```url
http[s]://server:port/ws2
```

Server and port will be the same as configured as attribute to webserver. which also must be enabled for the websocket interfaces to work.


## ws1 - original websocket interface :id=ws1-description

The protocol is a text based protocol that is simple and effective. Only **AUTH** and **NOOP** commands are valid in a system where the client has not been authenticated.

### Packet format :id=ws1-packet-format

Message traffic on the socket is in semicolon separated text format as of below

#### Format for commands :id=ws1-format-commands

 | 'C' ; command ; optional data that may be separated by additional semicolons. | 
 | ----------------------------------------------------------------------------- | 

Positive reply 

 | '+' ; originator | 
 | ---------------- |

Negative reply

 | '-' ; originator ; Error code ; Error in real text |
 | -------------------------------------------------- |

**Example:**
 | -;E;Transmit buffer full | 
 | ------------------------ | 

###  events :id=ws1-events

Both sent and received events have the same format.

 | 'E' ; head , vscp_class , vscp_type ,obid, datetime, timestamp, GUID, data | 
 | -------------------------------------------------------------------------- | 
    
    
Positive reply 

 | '+' | 
 | --- | 

Negative reply 

 | '-' | 
 | --- | 

** Example:**
```
E;0,30,5,0,2000-01-01T12:33:14,0,FF:FF:FF:FF:FF:FF:FF:FE:00:26:55:CA:00:06:00:00,0x01,0x01
```

 | Code | Description | 
 | :----: | ----------- | 
 | C    | Command. Parameter is command and optional parameter(s). | 
 | +    | Positive Reply. If send in response to a command the command is also  returned on the form '+;command' | 
 | -    | Negative reply. Payload is error code followed by error message in English. '-;2;Syntax Error' | 
 | E    | Event. Payload is VSCP event. | 

### Commands :id=ws1-commands

The following commands are currently available. **NOOP** is typically used to test if a connection is working. **OPEN**/**CLOSE** can be used to stop the stream from incoming events. Note that events still are collected at the server side and will be sent after a closed socket has been opened again. Use **CLRQUEUE** to clear the queue at the server before opening the stream if you don't want to receive collected events.

Variables on the server is a powerful tool. Variables are used by Level II drivers and the internal decision matrix of the daemon. This means that driver parameters can be changes and complex setups can be accomplished together with the decision matrix functionality. As an example you can set a variable to true and have an action executed by the daemon that does something useful. The possibilities are endless. Variables can be persistent meaning they will live over time so it can also be a method to save states.

#### NOOP :id=ws1-noop

No operation. Will always give a positive response.

```
C;NOOP
```

#### CHALLENGE :id=ws1-challenge

```
C;CHALLENGE
```

Send this command to initiate the authentication. This is process is normally started automatically on a connect.

####  AUTH :id=ws1-auth

This command is used by a client to authenticate itself. When connecting to a ws1 websocket the VSCP daemon will send something like

```
"+;AUTH0:5a475c082c80dcdf7f2dfbd976253b24" 
```

as a security challenge to the client (Also sent after a **CHALLENGE** command has been sent to a host by a client.). The "5a475c082c80dcdf7f2dfbd976253b24" is a session id or sid that is different for every connection.

The client now should send an authentication message on the form

```
"C;AUTH;sid;crypto"  
```

for example

```
C;AUTH;5a475c082c80dcdf7f2dfbd976253b24;69b1180d2f4809d39be34e19c750107f
```

where the sid is used as a 16.byte random iv for AES-128 encryption over

```
"username:password"
```

using the [vscpkey](./configuring_the_vscp_daemon.md#security) as a common secret key.  If the credentials are valid the server will respond with

```
"+;AUTH1;userid;name;password;fullname;filtermask;rights;remotes;events;note"
```

and if invalid the server will respond with

```
"-;8,Not authorized"
```

Note that in addition to the credentials the server IP address must also be valid as of the [configuration](./configuring_the_vscp_daemon.md) for this user.

**Note:** its is not allowed to have a username with a semicolon in the name.

The standard vscp password hash is calculated over _"username:authdomain:password"_. 

*rights* a bit array presented as eight bytes separated with semicolon.

#### OPEN :id=ws-open

Start receiving events. Collected events in queue will be sent. Positive/negative response is returned.

**Example:** 

```
“C;OPEN” 
```

positive response is 

```
“+;OPEN”
```

####  CLOSE :id=ws-close

Stop receiving events. Events are still collected in queue on server side. Positive/negative response is returned.

**Example:**

```
“C;CLOSE” 
```

positive response is 

```
“+;CLOSE”
```

####  CLRQUEUE / CLRQ  :id=ws-clrq

Clear events in input queue. Positive/negative response is returned.

**Example:**

```
“C;CLRQE” 
```

positive response is 

```
“+;CLRQ”
```

### SETFILTER / SF :id=ws-sf

Set filter/mask for incoming data. Positive/negative response is returned.

**Example:** 

```
“C;SF;filter-priority, filter-class, 
    filter-type, filter-GUID;mask-priority, 
    mask-class, mask-type, mask-GUID” 
```       

positive response is 

```
“+;SF”
```

Note that there is a semicolon between filter and mask information and commas between parts of each filter and mask.


### Send event :id=ws-send-event

The same format is used to send events as they are received.

 | 'E';head,vscp_class,vscp_type,obid,datetime,timestamp,GUID,data |
 | :--------------------------------------------------------------- |

and

 | +;EVENT |
 | :------- |

is received if it got sent and

 | -;EVENT;error-code;realtext-error | 
 | :--------------------------------- | 

is returned if there was a problem sending the event.

## Errors :id=ws1-error-codes

This table list the errors that currently is defined

 | Error code | Error message | 
 | :----------: | ------------- | 
 | 0 | No error | 
 | 1 | Syntax error. | 
 | 2 | Unknown command. | 
 | 3 | Transmit buffer full. | 
 | 4 | Problem allocating memory.| 
 | 5 | Not authorized. | 
 | 6 | Not authorized to send events. | 
 | 7 | Not allowed to do that. |
 | 8 | Parse error, invalid format. |
 | 9 | Unknown type, only know "COMMAND" and "EVENT". |

## ws2 - JSON baser websocket interface :id=ws2-description

The protocol is a JSON based protocol that is simple and effective. Only **AUTH** and **NOOP** commands are valid in a system where the client has not been authenticated.

### Packet format

Message traffic on the socket is on JSON format and there are a few different objects used.

### Command object :id=ws2-command-object

```json
{
    "type" : "C|CMD|COMMAND",
    "command" : "command",
    "args" : {
        arg-pairs or "null"
    },
    "error-code" : n,
    "error-str" : "error message"
}
```

This is the object a client send to execute a command. The _type_ can be set to either "C", "CMD" or "COMMAND".

_command_ should be set to the command to execute.

_args_ is either _null_ if the command has no arguments or a list of argument pairs if it have arguments.

Se AUTH command below for a command with arguments and the NOOP command for a command without arguments.

### Positive reply object :id=ws2-positive-reply-object


```json
{
    "type" : "+",
    "command" : "command",
    "args" : {
        ....
    }
}
```

A positive reply. _command_ tells which command the positive reply is associated with. The _args_ are optional arguments to the reply and is _null_ if there aree no arguments as in

```json
{
    "type" : "+",
    "command" : "command",
    "args" : null
}
```


### Negative reply object :id=ws2-negative-reply-object

```json
{
    "type" : "-",
    "command" : "command",
    "errcode" : n,
    "errstr" : "error message"
}
```

A negative reply. _command_ tells which command the negative reply is associated with. _error-code_ is set to a numerical error code describing the problem and _error-str_ is set to real text message that describe the error. [Error codes](./websocket_protocol_description.md#ws1-error-codes) are the same as for ws1 above.

### VSCP event object :id=ws2-event-object

This is either an event from a client to the VSCP daemon or an event from the daemon to the client. A client sending an event object will get a positive or negative reply object in return where command is set to "EVENT"

```json
{
    "type" : "EVENT",
    "event" :  {
       "vscpHead": 0,
       "vscpObId":  5,
       "vscpDateTime": "2020-01-27T20:47:55Z",
       "vscpTimeStamp": 3906069311,
       "vscpClass": 20,
       "vscpType": 3,
       "vscpGuid": "FF:FF:FF:FF:FF:FF:FF:F5:00:00:00:00:00:05:00:00",
       "vscpData": [15,14,13,12,11,10,9,8,7,6,5,4,3,2,0,0,1,35],
       "vscpNote": "An event note"
   }
}
```

### Commands :id=ws2-command

#### AUTH :id=ws2-command-auth

The client send the _AUTH_ command to create a session with the server. This session is active until the client disconnect from the websocket and can be over tls if needed.

Two arguments must be supplied

**iv** :id=ws2-commands-auth-sid
**crypto**

When connecting to a ws2 websocket the VSCP daemon will send something like

```
{    
    "type" : "+",
    "command": "AUTH0", 
    "args" : [
    	"AUTH0",
        "5a475c082c80dcdf7f2dfbd976253b24"
    ]
}
```

as a security challenge to the client (Also sent after a **CHALLENGE** command has been sent to a host by a client.). The "5a475c082c80dcdf7f2dfbd976253b24" here is a session id or sid that is random and different for every connection.

**crypto** :id=ws2-commands-auth-crypto

The sid argument is used as a 16 byte random iv for AES-128 encryption over

```
"username:password"
```

using the [vscpkey](./configuring_the_vscp_daemon.md#config-security-vscpkey) as a common **secret** key.  

If the credentials is valid the server will respond with

```
{ 
    "type" : "+",  
    "command" : "AUTH",
    "args" : null  
}

```

The sid received form the VSCP daemon is just there for convenience for clients that have hard to to create a 128 bit random number with enough entropy. If your client can come up with a good enough value you can use that value instead.

A client that want to create a new session should send an authentication message on the form

```
{
    "type": "cmd",
    "command": "auth",
    "args": {
        "iv":"5a475c082c80dcdf7f2dfbd976253b24",
        "crypto":657u0"69b1180d2f4809d39be34e19c750107f"
    }
}
```
If the credentials are valid the response will be

```
{ 
    "type" : "+",  
    "command" : "AUTH",  
}
```

and if invalid the server will respond with

```
{ 
    "type" : "-",  
    "command" : "AUTH",  
    "error-code" : 8,
    "error-str" : "Not authorized"
}
```
The error code/message may be different depending on the cause of the negative reply. See [possible error codes](#ws1-error-codes) above.

Note that in addition to the credentials the client ip address must also be valid as of the [configuration](./configuring_the_vscp_daemon.md#config-remote-user) allowfrom for this user.

#### CHALLENGE :id=ws2-command-challenge

```xml
{
  "type" : "CMD",
  "command" : "CHALLENGE"
  "args" : null
}
```

The challenge command can be sent to get the session id. The response is the same as the object received when connecting to the server

{
    "type" : "+",
    "command": "CHALLENGE", 
    "args" : {
        "sid" : "3ded39018dbf0e8e4512a7cac79fd487"
    }
}

#### OPEN :id=ws2-command-open

```xml
{
  "type" : "CMD",
  "command" : "OPEN"
  "args" : null
}
```

Open the communication channel. It is now possible to send events and incoming events will be received instead of just being queued. 

#### CLOSE :id=ws2-command-close

```xml
{
  "type" : "CMD",
  "command" : "CLOSE"
  "args" : null
}
```

Close the communication channel. It is no longer possible to send events after this has been done and incoming events will be queued.

#### SETFILTER / SF :id=ws2-command-setfilter

```xml
{
  "type" : "CMD",
  "command" : "SETFILTER"
  "args" : {
    "mask_priority" : number,
    "mask_class" : number,
    "mask_type" : number,
    "mask_guid" : "00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00",
    "filter_priority" : number,
    "filter_class" : number,
    "filter_type" : number,
    "filter_guid" : "00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00"
  }
}
```

Set filter/mask for the communication channel so the channel just receive the events that is of interest to the client.

The reply is either a positive or a negative reply object.

#### CLRQUEUE / CLRQ :id=ws2-command-clrqueue

```xml
{
  "type" : "CMD",
  "coommand" : "CLRQUEUE"
  "args" : null
}
```

Clear the incoming event queue for the communication channel.

The reply is either a positive or a negative reply object.

#### VERSION / VER :id=ws2-command-version

```xml
{
  "type" : "CMD",
  "coommand" : "VERSION"
  "args" : null
}
```

Get the version for the VSCP daemon.

The reply is the following object

```xml
{
  "type" : "+",
  "coommand" : "VERSION"
  "args" : {
      "version" : "major.minor.release-build"
  }
}
```


#### COPYRIGHT :id=ws2-command-copyright

```xml
{
  "type" : "CMD",
  "command" : "COPYRIGHT"
  "args" : null
}
```

Get copyright information. 

The reply is the following object

```xml
{
  "type" : "+",
  "command" : "COPYRIGHT"
  "args" : {
      "copyright" : "copyright text"
  }
}
```

### WS2 examples

#### HTML5

A simple ws2 interface connection example is [here](https://github.com/grodansparadis/vscp/blob/master/tests/websockets/ws2.html)

#### Python
Code that issue some commands, send an event and waiting for incoming events is [here](https://github.com/grodansparadis/vscp/blob/master/tests/websockets/test_ws2.py)

#### node.js
Code that issue some commands, send an event and waiting for incoming events is [here](https://github.com/grodansparadis/vscp/blob/master/tests/websockets/test_ws2.js)

[filename](./bottom_copyright.md ':include')





Created symlink /etc/systemd/system/multi-user.target.wants/vscpd.service → /lib
/systemd/system/vscpd.service.
Job for vscpd.service failed because the service did not take the steps required
 by its unit configuration.
See "systemctl status vscpd.service" and "journalctl -xe" for details.
Processing triggers for man-db (2.8.5-2) ...
N: Download is performed unsandboxed as root as file '/root/vscpd_14.0.5-4_amd64.deb' couldn't be accessed by user '_apt'. - pkgAcquire::Run (13: Permission denied)


No files in /usr/share/vscp/vscpd/certs/
