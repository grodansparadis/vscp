# Configuring the VSCP Daemon

The configuration file is used to tell which drivers should be used and how the VSCP daemon should operate. From version 15.0 this file has changed format from XML to JSON.

When you change something in the configuration file you have to restart the VSCP daemon for the changes to take effect. You do this with

```bash
sudo systemctl restart vscpd
```


## Location of the configuration file

The daemon needs a configuration file called **vscpd.json**. It will not start without this file nor if the file contains invalid information. Normally the installation program will create a default file.

On most machines it will be located in 

    /etc/vscp
    
The location that is searched to find the configuration file can be changed with switches when you start the daemon. see the section about [startup switches](http://www.vscp.org/docs/vscpd/doku.php?id=vscp_daemon_startup_switches) for each platform.


##  Description of the configuration :id=config-description

The configuration file is a standard JSON file that contains information that tells the VSCP daemon what to do and how it should be done. The information in it is divided into sections and this documentation and each section and it's content is described below.

You can view a sample configuration file [here](https://github.com/grodansparadis/vscp/blob/master/install_files/unix/vscpd.json).

## The general section :id=config-general

In the general section you find settings that are common to all components of the VSCP daemon software. 

```json
"runasuser" : "vscp",		
"guid" : "FF:FF:FF:FF:FF:FF:FF:F5:00:00:00:00:00:00:00:01",
"servername" : "The VSCP daemon",
"classtypedb" : "/var/lib/vscp/vscpd/vscp_events.sqlite3",
"maindb" : "/var/lib/vscp/vscpd/vscp.sqlite3",
"discoverydb" : "/var/lib/vscp/vscpd/vscp.sqlite3",
"vscpkey" : "/var/vscp/vscp.key",
"debug" : 0
```


### runasuser :id=config-general-runasuser

__Only on Unix/Linux__. User to run the VSCP daemon as. This is for security and folders for VSCP & friends are set up with permissions for a vscp user on install. By running on this user it is no need to run the VSCP daemon as the root user.

Change to the user you want or set to empty to run as the user who starts the VSCP daemon.


### guid :id=config-gerneral-guid

```xml
<guid>
    FF:FF:FF:FF:FF:FF:FF:FF:FF:FF:FF:F5:00:00:00:00
</guid>
```

Set the server GUID for the daemon. This GUID is the base for the system. 

If not set here (or all nills) a GUID will be formed from the (first) MAC address of the machine the daemon runs on (on Linux needs to be run as root to get this) or if this fails the local IP address is used to form the GUID. The GUID string should be in MSB -> LSB order and have the four lsb set to zero.

**Example**
    FF:EE:DD:CC:BB:AA:99:88:77:66:55:44:00:00:00:00

### debug :id=config-gerneral-debug    

The debug entry is a 64-bit number (each bit is a flag)  that enable a specific debugging capability of the VSCP daemon. If you have problem you should enable the relevant bits to be able to detect the cause for the problem.

TYhe debug bits are defined in [this file](https://github.com/grodansparadis/vscp/blob/master/src/vscp/common/vscp_debug.h).

Se the [solving problems](./solving_problems.md) section for more information.
 

### Servername :id=config-general-servername

If set this real text name will be used as an identifier for the server along with the GUID. The default name will be something like

    The VSCP Daemon

### classtypedb :id=config-general-classtypedb

This is a path to a sqlite3 database file that holds information about VSCP events and can give symbolic output from the VSCP daemon. You can for instance have topics that display class or type as token instead of the numerical code. Live this entry blank if you have no interest in this.

Events are added to the VSCP specification as times go by. The database installed with the VSCP daemon is the current at the moment of the release. If you want to update you can find the latest version [here](https://www.vscp.org/events/vscp_events.sqlite3). Update information is [here](https://www.vscp.org/events/version.json). 

Load the latest file with

```bash
cd /var/lib/vscp
sudo wget https://www.vscp.org/events/vscp_events.sqlite3
```

### classtypedb :id=config-general-maindb

THis is the main database file for the VSCP daemon. It's main content is the discovery database which is used to collect information about nodes in the system. 

This entry must point to a named file in a location that is writable (default is _/var/lib/vscp/vscpd/vscp.sqlite3_)

### classtypedb :id=config-general-vscpkey

This is the path to a security key that the VSCP daemon use to encrypt/decrypt information with. The default is _/var/vscp/vscp.key_ This file should only be editable by the root user and also not be possible to read by any one else.

----
##  MQTT :id=config-mqtt

This is the main MQTT settings of the VSCP daemon. This is where interface and discovery information will be published. Also subscribe topics are here for server commands.

Main MQTT settings are derived by drivers and other parts of the system from here. There is no ned to repeat information that is the same. Typically a driver, for example, will only differ in subscribe and publish tropics.

### host :id=config-mqtt-host

This is the tcp/ip address for the MQTT broker. Default is _127.0.0.1_, the local host.

### port :id=config-mqtt-port

This is the tcp/ip port for the MQTT broker. Default is 1883.

### user :id=config-mqtt-user

This is the username to use to login on the MQTT broker. Leave blank not to use credentials. 

### password :id=config-mqtt-password

This is the password to use to login on the MQTT broker. Leave blank not to use credentials. 

### format :id=config-mqtt-format

This is the format the VSCP daemon will use to publish it's information. There are four formats available

| Format | Description |
| ------ | ----------- |
| json | This is the default. JSON format will be used. |
| xml  | XML format will be used. |
| string | Comma separated string format will be used. |
| binary | Binary format will be used. |

Default is JSON.

### qos :id=config-mqtt-qos

Quality of service for the MQTT connection. Default is zero. 

| qos | Description |
| --- | ----------- |
| 0 | The event is sent at most once. |
| 1 | The event is sent at least once. |
| 2 | The event is sent at exactly once. |

You can read more about qos [here](https://www.hivemq.com/blog/mqtt-essentials-part-6-mqtt-quality-of-service-levels/).

### bcleansession :id=config-mqtt-bcleansession

If true, start a clean MQTT session. If not set events are saved if the client disconnect from a broker and is delivered when it connects again. The client is identified by its client id.

You can read more about clean MQTT sessions [here](https://www.hivemq.com/blog/mqtt-essentials-part-7-persistent-session-queuing-messages/).

### bretain :id=config-mqtt-bretain

If true, the **last** VSCP event is stored along with the qos for that event. Each client that subscribes to a topic pattern that matches the topic of the retained message receives the retained message immediately after they subscribe. You can read more [here](https://www.hivemq.com/blog/mqtt-essentials-part-8-retained-messages/).

Default is false.

### keepalive :id=config-mqtt-keepalive

This is the time in seconds between keepalive messages. det default is sixty seconds.

### reconnect-delay :id=config-mqtt-reconnect-delay

If a connection is broken this is the time in seconds the client will wait until it tries to connect the MQTT broker again.

### reconnect-delay-max :id=config-mqtt-reconnect-delay-max

This is the maximum time a client will wait until it reconnects to a MQTT broker.

### reconnect-reconnect-exponential-backoff :id=config-mqtt-reconnect-exponential-backoff

Enable exponential back off for the reconnection. This means the reconnection attempts will be slower and slower.

**Example 1**

delay=2, delay_max=10, exponential_backoff=False Delays would be: 2, 4, 6, 8, 10, 10, ...

**Example 2**

delay=3, delay_max=30, exponential_backoff=True Delays would be: 3, 6, 12, 24, 30, 30, ...

### topic-interfaces :id=config-mqtt-topic-interfaces

On this topic interfaces of the VSCP daemon are published as a retained message.

### subscribe :id=config-mqtt-subscribe

This an array of topics on which the VSCP daemon will publish it's events. You can specify as many as you like.  Mustache escapes (escape tokens within {{_token_}}) can be used to create dynamic topics. The following escapes are valid for subscribe topics

| Escape | Description |
| ------ | ----------- |
| {{guid}} | Will be replaces with the GUID of the VSCP daemon. |

For example

```bash
"vscp/{{guid}}/#"
```

will subscribe to 

```bash
"vscp/FF:FF:FF:FF:FF:FF:FF:FF:FF:FF:FF:F5:00:00:00:00/#"
```


### publish :id=config-mqtt-publish

This is an array of topics which the VSCP daemon will subscribe to. You can specify as many as you like. Mustache escapes (escape tokens within {{_token_}}) can be used to create dynamic topics. The topic is created just before the event is published. The following escapes are valid for publish topics

| Escape | Description |
| ------ | ----------- |
| {{guid}} | Will be replaces with the GUID of the VSCP event that is about to be sent to the MQTT broker. |
| {{guid-id}} | Will be replaces with the GUID id from the discovery database or the GUID if not present. |
| {{guid[n]}} | Will be replaces with the GUID digit n of the VSCP event that is about to be sent to the MQTT broker. |
| {{guid.msb}} | Will be replaces with the GUID most significant byte of the VSCP event that is about to be sent to the MQTT broker. |
| {{guid.lsb}} | Will be replaces with the GUID least significant byte of the VSCP event that is about to be sent to the MQTT broker. |
| {{ifguid}} | Will be replaces with the interface GUID for a driver or with the VSCP daemon GUID for server originating events. |
 |
| {{ifguid[n]}} | Will be replaces with the n:th digit of the interface GUID for a driver or with the n:th digit of the VSCP daemon GUID for server originating events. |
| {{nickname}} | Will be replaced with the nickname. An unsigned integer formed of the least significant bytes of the event GUID. |
| {{data[n]}} | Will be replaces with the n:th data byte of the VSCP event that is about to be sent to the MQTT broker. A dash will be set if n is larger then the number of data bytes in the event. |
| {{class}} | The class of the VSCP event that is about to be sent to the MQTT broker. |
| {{type}} | The type of the VSCP event that is about to be sent to the MQTT broker.  |
| {{class-token}} | The class token of the VSCP event that is about to be sent to the MQTT broker. Needs the vscp event database. |
| {{type-token}} | The type-token of the VSCP event that is about to be sent to the MQTT broker. Needs the vscp event database. |
| {{head}} | The _head_ of the VSCP event that is about to be sent to the MQTT broker.  |
| {{obid}} | The _obid_ of the VSCP event that is about to be sent to the MQTT broker.  |
| {{timestamp}} | The _timestamp_ of the VSCP event that is about to be sent to the MQTT broker.  |
| {{dt}} | The _datetime_ string of the VSCP event that is about to be sent to the MQTT broker.  |
| {{year}} | The _year_ of the datetime of the VSCP event that is about to be sent to the MQTT broker.  |
| {{month}} | The _month_ of the datetime of the VSCP event that is about to be sent to the MQTT broker.  |
| {{day}} | The _day_ of the datetime of the VSCP event that is about to be sent to the MQTT broker.  |
| {{hour}} | The _hour_ of the datetime of the VSCP event that is about to be sent to the MQTT broker.  |
| {{minute}} | The _minute_ of the datetime of the VSCP event that is about to be sent to the MQTT broker.  |
| {{second}} | The _second_ of the datetime of the VSCP event that is about to be sent to the MQTT broker.  |
| {{clientid}} | The MQTT client id for the current connection. |
| {{user}} | The _user_ of the the current MQTT connection. |
| {{host}} | The _host_ of the the current MQTT connection. |

**Example 1**

```bash
"vscp/{{guid}}/{{class}}/{{type}}/{{nickname}}"
```

will be 

```bash
vscp/FF:FF:FF:FF:FF:FF:FF:FE:5C:CF:7F:C4:1E:7B:00:08/10/6/8
```

if the event's GUID is FF:FF:FF:FF:FF:FF:FF:FE:5C:CF:7F:C4:1E:7B:00:08 and the VSCP class is 10 and the VSCP type is 6, that is a temperature measurement event.

**Example 2**

```bash
"vscp/{{guid}}/{{class-token}}/{{type-token}}/{{nickname}}"
```

will be 

```bash
vscp/FF:FF:FF:FF:FF:FF:FF:FE:5C:CF:7F:C4:1E:7B:00:08/CLASS1.MEASUREMENT/VSCP_TYPE_MEASUREMENT_TEMPERATURE/8
```

if the event's GUID is FF:FF:FF:FF:FF:FF:FF:FE:5C:CF:7F:C4:1E:7B:00:08 and the VSCP class is 10 and the VSCP type is 6, that is a temperature measurement event.


### SSL/TLS support

### cafile :id=config-mqtt-cafile

Path to a file containing the PEM encoded trusted CA certificate files.  Either cafile or capath must not be NULL if a secure connection is required.

### capath :id=config-mqtt-capath

Path to a directory containing the PEM encoded trusted CA certificate files.  See [mosquitto.conf](https://mosquitto.org/man/mosquitto-conf-5.html) for more details on configuring this directory.  Either cafile or capath must not be NULL if a secure connection is required.

### certfile :id=config-mqtt-certfile

Path to a file containing the PEM encoded certificate file for this client.  If NULL, keyfile must also be NULL and no client certificate will be used.

### keyfile :id=config-mqtt-keyfile

Path to a file containing the PEM encoded private key for this client.  If NULL, certfile must also be NULL and no client certificate will be used.

----
##  Level I Drivers :id=config-level1-driver

Define a VSCP daemon level I driver. If disabled the driver will be loaded. 

Level I drivers was in the past called CANAL (CAN Abstraction Layer) drivers.

### Assigning GUID's to the interfaces of a VSCP daemon

You are free to pick any GUID you like as long as it (normally) have the two least significant bytes of the GUID available for nodes on the interface to be used as there nicknames. Best and simplest is to select GUID's based on the nodes IP address. That is

IP:  FF:FF:FF:FF:FF:FF:FF:FD:YY:YY:YY:YY:XX:XX:XX:XX

YY... is the IP address. 

For a multi driver/interface machine setup, the IP address is the best choice as the user available range XX:XX:XX:XX have room for many interfaces. For instace one can assign 00:00:XX:XX to the first, 00:01:XX:XX to the second etc. Here XX:XX is used for node nicknames. So driver 1 will have GUID

FF:FF:FF:FF:FF:FF:FF:FD:YY:YY:YY:YY:00:00:00:00

and driver 2

FF:FF:FF:FF:FF:FF:FF:FD:YY:YY:YY:YY:00:01:00:00

and so on.

```json
"drivers" : {
    "level1" : [
        {
            "enable" : false,
            "name" : "logger",
            "config" : "/tmp/canallog.txt",
            "flags" : 1,
            "translation" : 2,
            "path" : "/var/lib/vscp/drivers/level1/vscpl1drv-logger.so",
            "guid" : "FF:FF:FF:FF:FF:FF:FF:F5:01:00:00:00:00:00:00:01",

            "mqtt" : {
                "host" : "127.0.0.1",
                "port" : 1883,
                "user" : "vscp",
                "password": "secret",
                "clientid" : "mosq-l1drv-logger",
                "format" : "json",
                "qos" : 0,
                "bcleansession" : false,
                "bretain" : false,
                "keepalive" : 60,
                "reconnect-delay" : 2,
                "reconnect-delay-max" : 10,
                "reconnect-exponential-backoff" : false,
                "cafile" : "",
                "capath" : "",
                "certfile" : "",
                "keyfile" : "",
                "pwkeyfile" : "",
                "subscribe" : [
                    "vscp/{{guid}}/#"
                ],
                "publish" : [
                    "vscp/{{guid}}/->/{{class}}/{{type}}/{{nodeid}}"
                ]
            }
        }
    ]
}
```

The content consist of one or several driver entries in an array each representing a level 1 driver that should be used. 

### enable :id=config-level1-driver-enable

Set to true to enable loading of the driver, set to false to disable.

### name :id=config-level1-driver-name

A name given by the user for the driver. This is the name by which the driver is identified by the system.

### config :id=config-level1-driver-config

The semicolon separated configuration string for the driver. The meaning of this string is driver specific. See documentation for the driver.

### flags :id=config-level1-driver-flags

A 32 bit driver specific number that have bits set that have special meaning for a driver. See the documentation for a specific driver for an explanation of the meaning of each flag bit for that driver.

### path :id=config-level1-driver-path

Path to where the driver is located. Usually points to a file located in _/var/lib/vscp/drivers/level1/_

### guid :id=config-level1-driver-guid

__Must be set to a unique and valid GUID.**

The GUID that the driver will use for it's interface. This means that this is the GUID that will be the source of Level I events.

Note that the GUID set here should have the two least significant bytes set to zero as they are reserved for nickname id's.

### Translation :id=config-level1-driver-translation

This is a list of semicolon separated fields with translations that should be carried out by the system on event passing through driver.

 | Mnemonic  | Description  | 
 | :--------:  | -----------  | 
 | Bit 1 (1) | Incoming Level I measurement events is translated to Level II measurement, floating point events. | 
 | Bit 2 (2) | Incoming Level I measurement events is translated to Level II measurement, string events.  | 
 | Bit 3 (4) | All incoming events will be translated to class Level I events over Level II, that is have 512 added to it's class. | 


### MQTT :id=config-level1-driver-mqtt

This is the same MQTT information described above. You can leave out parts that are the same in which case the main settings will be used. Or you can repeat the information or set new.

The _subscribe_ and _publish_ array's are the only parts that **must be preset**.



----

## Level II drivers :id=config-level2-driver

Level II drivers can handle the full VSCP abstraction and don't have the small payload size and other limitations of the Level I drivers.

### Assigning GUID's to the interfaces of a VSCP daemon

You are free to pick any GUID you like as long as it (normally) have the two least significant bytes of the GUID available for nodes on the interface to be used as there nicknames. Best and simplest is to select GUID's based on the nodes IP address. That is

IP:  FF:FF:FF:FF:FF:FF:FF:FD:YY:YY:YY:YY:XX:XX:XX:XX

YY... is the IP address. 

For a multi driver/interface machine setup, the IP address is the best choice as the user available range XX:XX:XX:XX have room for many interfaces. For instace one can assign 00:00:XX:XX to the first, 00:01:XX:XX to the second etc. Here XX:XX is used for node nicknames. So driver 1 will have GUID

FF:FF:FF:FF:FF:FF:FF:FD:YY:YY:YY:YY:00:00:00:00

and driver 2

FF:FF:FF:FF:FF:FF:FF:FD:YY:YY:YY:YY:00:01:00:00

and so on.

```json
"drivers" : {
    "level1" : [
        {
            "enable" : false,
            "name" : "tcpiplink",
            "path-driver" : "/var/lib/vscp/drivers/level2/vscpl2drv-tcpiplink.so",
            "path-config" : "/var/lib/vscp/vscpd/tcpiplink.conf",
            "guid" : "FF:FF:FF:FF:FF:FF:FF:F5:09:00:00:00:00:00:00:00",

            "mqtt" : {
                "host" : "127.0.0.1",
                "port" : 1883,
                "user" : "vscp",
                "password": "secret",
                "clientid" : "mosq-vscp-daemon-000001",
                "format" : "json",
                "qos" : 0,
                "bcleansession" : false,
                "bretain" : false,
                "keepalive" : 60,
                "reconnect-delay" : 2,
                "reconnect-delay-max" : 10,
                "reconnect-exponential-backoff" : false,
                "cafile" : "",
                "capath" : "",
                "certfile" : "",
                "keyfile" : "",
                "pwkeyfile" : "",
                "subscribe" : [
                    "vscp/{{guid}}/#"
                ],
                "publish" : [
                    "vscp/{{guid}}/->/{{class}}/{{type}}/{{nodeid}}"
                ]
            }
        }
    ]
}
```

### enable :id=config-level2-driver-enable

The driver should be loaded if set to true. If false the driver will not be loaded.

### name :id=config-level2-driver-name

A name given by the user for the driver.

### path-driver :id=config-level2-driver-path-driver

This is the path to the driver. Usually points to a file located in _/var/lib/vscp/drivers/level2/_

### path-config :id=config-level2-driver-path-config

This is the path to the driver configuration file. This file can be located in /etc/vscp for best security or in _/var/lib/vscp/_ if the HLO write ability should be used or in any other suitable place on your disk.

The format of this file is up to the driver creator. JSON format is most common but XML and other formats are also valid.

###  guid :id=config-level2-driver-guid

__Must be set to a unique and valid GUID.**

The GUID that the driver will use for it's interface.


[filename](./bottom_copyright.md ':include')
