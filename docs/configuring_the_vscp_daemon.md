# Configuring the VSCP Daemon

The configuration file is used to tell which drivers should be used and how the VSCP daemon should operate. From version 15.0 this file has changed format from XML to JSON.

As always with configuration files it easy to make changes that make the system nonfunctional. Sp be careful when you edit the file and keep to the JSON standard. Most important keep a backup so that yo can go back to a working copy if something goes wrong.

It is very convenient to use one of the online JSON validators to validate the JSON file when doing a lot of changes. There are many available and a good one is [this one](https://jsonformatter.curiousconcept.com/). Just copy the content of the file in the box and validate.

When you change something in the configuration file you have to restart the VSCP daemon for the changes to take effect. You do this with

```bash
sudo systemctl restart vscpd
```

If something is wrong the VSCP daemon may not start. Check the log file at **/var/log/vscpd.log**. You can also use

```bash
ps aux | grep vscpd
```

to see if the VSCP daemon is running or not.



## Location of the configuration file

The daemon needs a configuration file called **vscpd.json**. The server will not start without this file nor if the file contains invalid information or information on invalid format. Normally the installation program will create a default file.

On most machines the configuration file will be located in 

    /etc/vscp/vscpd.json
    
The location that is searched to find the configuration file can be changed with switches when you start the daemon. see the section about [startup switches](http://www.vscp.org/docs/vscpd/doku.php?id=vscp_daemon_startup_switches) for each platform.

## To think about before you start :id=think-before

### GUID's :id=think-before-guid

GUID's stands for **G**lobally **U**nique **ID**entifiers and this id is used in VSCP to identify thing. All devices and nodes are identified by a GUID but so is also the VSCP daemon (it is after all a node to) and each of the drivers connected to the server. Check the [VSCP specification](https://docs.vscp.org/#vscpspec) for more information about GUID's.

The daemon needs a GUID assigned to it. Preferably a unique one. It is possible to base a GUID on the machines MAC address if it have an ethernet interface. There is also other id's to build upon. One can also ask for a personal assigned series. See [the VSCP specification](https://docs.vscp.org/#vscpspec) for more info on how.

For most users the MAC address of the machine the VSCP daemon is installed on is the best solution. The installation package will even install a helper script on Linux that help you create this GUID. The script is called **vscp_eth_tp_guid**. To used it you need the name of your ethernet interface. You can get that with

```bash
ip link show
```
Say that your ethernet interface is **eth0** then you can use the script like

```bash
vscp_eth_to_guid eth0
```
and you will get a valid GUID from your ethernet address. You can even use

```bash
sudo vscp_eth_to_guid eth0 /etc/vscp/vscpd.json
```
and the distributed demo GUID's set in the standard file will all be changed.

##  Description of the configuration :id=config-description

The configuration file is a standard JSON file that contains information that tells the VSCP daemon what to do and how it should be done. The information in it is divided into sections and this documentation and each section and it's content is described below.

You can view a sample configuration file [here](https://github.com/grodansparadis/vscp/blob/master/resources/linux/vscpd.json).

## The general section :id=config-general

In the general section you find settings that are common to all components of the VSCP daemon software. 

```json
"runasuser" : "vscp",	
"debug" : 0,	
"guid" : "FF:FF:FF:FF:FF:FF:FF:F5:00:00:00:00:00:00:00:01",
"servername" : "The VSCP daemon",
"classtypedb" : "/var/lib/vscp/vscpd/vscp_events.sqlite3",
"maindb" : "/var/lib/vscp/vscpd/vscp.sqlite3",
"discoverydb" : "/var/lib/vscp/vscpd/vscp.sqlite3",
"vscpkey" : "/var/vscp/vscp.key",

"logging" : {
    "file-enable-log": true,
    "file-log-level" : "debug",
    "file-pattern" : "[vscp] [%^%l%$] %v",
    "file-path" : "/var/log/vscp/vscpd.log",
    "file-max-size" : 5242880,
    "file-max-files" : 7,
    "console-enable-log": false,
    "console-log-level" : "info",
    "console-pattern" : "[vscp] [%^%l%$] %v"
}
```

### debug :id=config-gerneral-debug    

The debug entry is a 64-bit number (each bit is a flag)  that enable a specific debugging capability of the VSCP daemon. If you have problem you should enable the relevant bits to be able to detect the cause for the problem.

The debug bits are defined in [this file](https://github.com/grodansparadis/vscp/blob/master/src/vscp/common/vscp_debug.h).

Se the [solving problems](./solving_problems.md) section for more information.

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
```
    FF:EE:DD:CC:BB:AA:99:88:77:66:55:44:00:00:00:00
```

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

### maindb :id=config-general-maindb
THis is the main database file for the VSCP daemon. It's main content is the discovery database which is used to collect information about nodes in the system. 

This entry must point to a named file in a location that is writable (default is _/var/lib/vscp/vscpd/vscp.sqlite3_)

### vscpkey :id=config-general-vscpkey
This is the path to a security key that the VSCP daemon use to encrypt/decrypt information with. The default is _/var/vscp/vscp.key_ This file should only be editable by the root user and also not be possible to read by any one else.

### debuglevel :id=config-general-debug-level
This is the debug level. Zero is no debugging. A higher number is different levels of debugging detail.

### logging :id=config-general-logging
This is the log console and file settings. Default is to log to the logfile */var/log/vscp/vscpd.log* on Linux and *\program files (x86)\vscp\vscp.log* on Windows. The exception is at vscpd start up, before the configuration file is read and the configured location for the log file is unknown, here logging is done to the console and on Linux also to the syslog.

Modes for logging can be set as of below. In debug/trace mode the debug flag above defines how much info is logged.

#### Logging to file

##### file-enable-log :id=config-general-logging-file-enable-log
Enable logging to a file by setting to *true*.

##### file-enable-log :id=config-general-logging-file-enable-log
Log level for file log. Default is "info".

| Level | Description |
| ----- | ----------- |
| "trace" | Everything is logged |
| "debug" | Everything except trace is logged |
| "info" | info and above is logged |
| "err" | Errors and above is logged |
| "critical" | Only critical messages are logged |
| "off" | No logging |

##### file-pattern :id=config-general-logging-file-pattern
Log file pattern as described [here](https://github.com/gabime/spdlog/wiki/3.-Custom-formatting).

##### file-path :id=config-general-logging-file-path

Path to log file. Default is on Linux is */var/log/vscp/vscpd.log*

##### file-max-size :id=config-general-logging-file-max-size
Max size for log file. It will be rotated if over this size. Default is 5 Mb.

##### file-max-files :id=config-general-logging-file-max-files
Maximum number of log files to keep. Default is 7.

#### Logging to console

##### console-enable-log :id=config-general-logging-console-enable-log
Enable logging to a console by setting to *true*.

##### console-log-level :id=config-general-logging-console-log-level
Log level for console log. Default is "info".

| Level | Description |
| ----- | ----------- |
| "trace" | Everything is logged |
| "debug" | Everything except trace is logged |
| "info" | info and above is logged |
| "err" | Errors and above is logged |
| "critical" | Only critical messages are logged |
| "off" | No logging |

##### console-pattern :id=config-general-logging-console-pattern

Format for consol log.

----
##  MQTT :id=config-mqtt

This is the main MQTT settings of the VSCP daemon. This is where interface and discovery information will be published. Also subscribe topics are here for server information and commands.

Main MQTT settings are derived by drivers and other parts of the system from here. There is no need to repeat information that is the same. Typically a driver, for example, will only differ in subscribe and publish tropics.

```json
"bind" : "",   
"host" : "test.mosquitto.org",
"port" : 1883,
"mqtt-options" : {
    "tcp-nodelay" : true,
    "protocol-version": 311,
    "receive-maximum": 20,
    "send-maximum": 20,
    "ssl-ctx-with-defaults": 0,
    "tls-ocsp-required": 0,
    "tls-use-os-certs" : 0
},
```
### bind :id=config-mqtt-bind
If you want to bind to a specific interface on your machine enter it's address here. 

### host :id=config-mqtt-host
This is the tcp/ip address for the MQTT broker. Default is _127.0.0.1_, the local host.

### port :id=config-mqtt-port
This is the tcp/ip port for the MQTT broker. Default is 1883.

### **mqtt-options** :id=config-mqtt-options
Set special MQTT connect option here.

### tcp-nodelay :id=config-mqtt-options-tcp-nodelay
Disable [Nagle's algorithm](https://en.wikipedia.org/wiki/Nagle%27s_algorithm) by setting to *true*. This will give more traffic but is anyway preferred for VSCP's small sized events. Setting to *true* disables Nagle's algorithm (default), *false* enables.

### protocol-version :id=config-mqtt-options-protocol-version
The protocol version of MQTT to use. Valid values are 310 for version 3.10 and 311 (default) for version 3.11 and 500 for version 5.

### receive-maximum :id=config-mqtt-options-receive-maximum
Value can be set between 1 and 65535 inclusive, and represents the maximum number of incoming QoS 1 and QoS 2 messages that this client wants to process at once.  Defaults to 20.  This option is not valid for MQTT v3.1 or v3.1.1 clients.  Note that if the MQTT_PROP_RECEIVE_MAXIMUM property is in the proplist passed to mosquitto_connect_v5(), then that property will override this option.  Using this option is the recommended method however.

### send-maximum :id=config-mqtt-options-send-maximum
Value can be set between 1 and 65535 inclusive, and represents the maximum number of outgoing QoS 1 and QoS 2 messages that this client will attempt to have “in flight” at once.  Defaults to 20.  This option is not valid for MQTT v3.1 or v3.1.1 clients.  Note that if the broker being connected to sends a MQTT_PROP_RECEIVE_MAXIMUM property that has a lower value than this option, then the broker provided value will be used.

### ssl-ctx-with-defaults :id=config-mqtt-options-ssl-ctx-with-defaults
If value is set to a non zero value, then the user specified SSL_CTX passed in using MOSQ_OPT_SSL_CTX will have the default options applied to it.  This means that you only need to change the values that are relevant to you.  If you use this option then you must configure the TLS options as normal, i.e.  you should use mosquitto_tls_set to configure the cafile/capath as a minimum.  This option is only available for openssl 1.1.0 and higher.

### tls-ocsp-required :id=config-mqtt-options-tls-ocsp-required
Set whether OCSP checking on TLS connections is required.  Set to 1 to enable checking, or 0 (the default) for no checking.

### tls-use-os-certs :id=config-mqtt-options-tls-use-os-certs
Set to 1 to instruct the client to load and trust OS provided CA certificates for use with TLS connections.  Set to 0 (the default) to only use manually specified CA certs.

```json
"user" : "vscp",
"password": "secret",
"clientid" : "the-vscp-daemon",  
"publish-format" : "json",
"subscribe-format" : "auto",
"qos" : 1,
"bcleansession" : false,
"bretain" : false,      
"keepalive" : 60,
"bjsonmeasurementblock": true,
"topic-daemon-base": "vscp-daemon/{{guid}}/",
"topic-interfaces" : "interfaces",
"topic-discovery" : "discovery",
"reconnect" : {
    "delay" : 2,
    "delay-max" : 10,
    "exponential-backoff" : false
},
```

### user :id=config-mqtt-user
This is the username to use to login on the MQTT broker. Leave blank to use no credentials. 

### password :id=config-mqtt-password
This is the password to use to login on the MQTT broker. Leave blank to use no credentials. 

### clientid :id=config-mqtt-clientid
Set the client id of this client

### publish-format :id=config-mqtt-general-publish-format
This is the format the VSCP daemon will use to publish it's information if niot set specially for the publish topic. There are four formats available

| Format | Description |
| ------ | ----------- |
| json | JSON formatted payload will be published. |
| xml  | XML formatted payload will be published. |
| string | Comma separated string formatted payload will be published. |
| binary | Binary formatted payload will be published. |

Default is *json*.

### subscribe-format :id=config-mqtt-general-subscribe-format
This is the format the VSCP daemon will expect subscriptions to ne formatted if not set specially for the subscribe topic. There are five formats available

| Format | Description |
| ------ | ----------- |
| auto | The VSCP daemon will try to detect the payload format. |
| json | JSON formatted payload will be expected. |
| xml  | XML formatted paylaod will be expected. |
| string | Comma separated string formatted payload will be expected. |
| binary | Binary formatted payload will be expected. |

Default is *auto*.

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

Default is *false*.

### keepalive :id=config-mqtt-keepalive
This is the time in seconds between keepalive messages. Default is sixty seconds.

### bjsonmeasurementblock :id=config-mqtt-bjsonmeasurementblock
If set to true published VSCP will get the measurement JSON data added to the event. Information about the format is [here](https://grodansparadis.github.io/vscp-doc-spec/#/./vscp_level_ii_specifics?id=json-representation).

### topic-daemon-base :id=config-mqtt-topic-daemon-base
This is the first part of the base topic and it will be prefix for other topics such as *drivers* and *discovery*.

The VSCP daemon will publish some information under this topic.

Default is *vscp-daemon({{guid}}/*

### topic-drivers :id=config-mqtt-topic-drivers
This setting together with **topic-daemon-base** as prefix will construct the topic for driver publishing.

Default is *drivers* whish will give the topic *vscp-daemon({{guid}}/drivers*

### topic-discovery :id=config-mqtt-topic-discovery
This setting together with **topic-daemon-base** as prefix will construct the topic for discovery publishing.

Default is *discovery* whish will give the topic *vscp-daemon({{guid}}/discovery*

### **reconnet**

### delay :id=config-mqtt-reconnect-delay
If a connection is broken this is the time in seconds the client will wait until it tries to connect the MQTT broker again.

### delay-max :id=config-mqtt-reconnect-delay-max
This is the maximum time a client will wait until it reconnects to a MQTT broker.

### exponential-backoff :id=config-mqtt-reconnect-exponential-backoff
Enable exponential back off for the reconnection. This means the reconnection attempts will be slower and slower.

**Example 1**

delay=2, delay_max=10, exponential_backoff=False Delays would be: 2, 4, 6, 8, 10, 10, ...

**Example 2**

delay=3, delay_max=30, exponential_backoff=True Delays would be: 3, 6, 12, 24, 30, 30, ...

### **SSL/TLS support**

```json
"tls" : {
    "cafile" : "",
    "capath" : "",
    "certfile" : "",
    "keyfile" : "",
    "pwkeyfile" : "",
    "no-hostname-checking" : true,
    "cert-reqs" : 0,
    "version": "",
    "ciphers": "",
    "psk": "",
    "psk-identity" : ""
},
```

### cafile :id=config-mqtt-cafile
Path to a file containing the PEM encoded trusted CA certificate files.  Either cafile or capath must not be NULL if a secure connection is required.

### capath :id=config-mqtt-capath
Path to a directory containing the PEM encoded trusted CA certificate files.  See [mosquitto.conf](https://mosquitto.org/man/mosquitto-conf-5.html) for more details on configuring this directory.  Either cafile or capath must not be NULL if a secure connection is required.

### certfile :id=config-mqtt-certfile
Path to a file containing the PEM encoded certificate file for this client.  If NULL, keyfile must also be NULL and no client certificate will be used.

### keyfile :id=config-mqtt-keyfile
Path to a file containing the PEM encoded private key for this client.  If NULL, certfile must also be NULL and no client certificate will be used.

### pwkeyfile :id=config-mqtt-pwkeyfile
path to a file containing the PEM encoded private key for this client.  If NULL, certfile must also be NULL and no client certificate will be used.

### no-hostname-checking :id=config-mqtt-no-hostname-checking
if set to false, the default, certificate hostname checking is performed.  If set to true, no hostname checking is performed and the connection is insecure.


### cert-reqs :id=config-mqtt-cert-reqs
an integer defining the verification requirements the client will impose on the server.  This can be one of:
SSL_VERIFY_NONE (0): the server will not be verified in any way.
SSL_VERIFY_PEER (1): the server certificate will be verified and the connection aborted if the verification fails.  The default and recommended value is SSL_VERIFY_PEER.  Using SSL_VERIFY_NONE provides no security.

### version :id=config-mqtt-version
The version of the SSL/TLS protocol to use as a string.  If NULL, the default value is used.  The default value and the available values depend on the version of openssl that the library was compiled against.  For openssl >= 1.0.1, the available options are tlsv1.2, tlsv1.1 and tlsv1, with tlv1.2 as the default.  For openssl < 1.0.1, only tlsv1 is available.

### ciphers :id=config-mqtt-ciphers
A string describing the ciphers available for use.  See the “openssl ciphers” tool for more information.  If NULL, the default ciphers will be used.

### psk :id=config-mqtt-psk
Configure the client for pre-shared-key based TLS support. the pre-shared-key in hex format with no leading “0x”.


### psk-identity :id=config-mqtt-psk-identity
The identity of this client.  May be used as the username depending on the server settings.

### **Last Will**

Configure will information for a mosquitto instance.  By default, clients do not have a will.  

```json
"will": {
    "topic": "vscp-daemon/{{guid}}/will",
    "qos": 1,
    "retain": true,
    "payload": "VSCP Daemon is down"
},
```

### topic:id=config-mqtt-will-topic
This is the topic for the will. Default is _vscp-daemon/{{guid}}/will_

### qos :id=config-mqtt-will-qos
Integer value 0, 1 or 2 indicating the Quality of Service to be used for the will. Default is _1_

### retain :id=config-mqtt-will-retain
Set to true to make the will a retained message. Default is _true_

### payload :id=config-mqtt-will-payload
Payload to use for will. Default is _VSCP Daemon is down_

### **subscribe** :id=config-mqtt-subscribe

This an array of topics on which the VSCP daemon will get it's incoming VSCP events. You can specify as many as you like.  Mustache escapes (escape tokens within {{_token_}}) can be used to create dynamic topics.

```json
"subscribe": [
    {
        "topic": "vscp/topic/A",
        "qos": 0,
        "v5-options": 0,
        "format": "auto"
    },
    {
        "topic": "vscp/topic/B",
        "qos": 0,
        "v5-options": 0,
        "format": "auto"
    },
    {
        "topic": "vscp/los/#",
        "qos": 0,
        "v5-options": 0,
        "format": "auto"
    },
    {
        "topic": "vscp/users/#",
        "qos": 0,
        "v5-options": 0,
        "format": "auto"
    }
],
```

See the [subscribe](./subscribe_server.md) page for more information.

### topic :id=config-mqtt-subscribe-topic
Subscribe topic

### qos :id=config-mqtt-subscribe-qos
Integer value 0, 1 or 2 indicating the Quality of Service to be used for the will. Default is _0_

### v5-options :id=config-subscribe-will-v5-options
MQTT version 5 options. See information [here](https://mosquitto.org/api/files/mqtt_protocol-h.html#mqtt5_sub_options)

### format :id=config-mqtt-subscribe-format
If not set the [config-mqtt-subscribe-format](./configuring_the_vscp_daemon.md#config-mqtt-general-subscribe-format) subscribe format will be used. 

| Format | Description |
| ------ | ----------- |
| auto | The VSCP daemon will try to detect the payload format. |
| json | JSON formatted payload will be expected. |
| xml  | XML formatted paylaod will be expected. |
| string | Comma separated string formatted payload will be expected. |
| binary | Binary formatted payload will be expected. |

## User escapes for topics :id=config-user-escapes

User escapes is a possibility to define your own dynamic escapes for published topics. The consist of two parts. A key and a value. Both are on string form. They work such that a {{'key'}} set in a topic will be replaced by {{'value'}} before the message is published.

See [server publishing](./publishing_server.md) for more information.

```json
"bescape-pub-topics": true,
"user-escapes": {
    "escape1": "value1",
    "escape2": "value2"
},
```

### bescape-pub-topics :id=config-user-escapes-bescape-pub-topics
Set to *true* to enable the functionality.

### user-escapes:id=config-user-escapes-bescape-map
Here any number of key/value pairs can be defined.

### publish :id=config-mqtt-publish

This is an array of topics which the VSCP daemon will subscribe to. You can specify as many as you like. Mustache escapes (escape tokens within {{_token_}}) can be used to create dynamic topics. The topic is created just before the event is published. The following escapes are valid for publish topics

``` json
"publish": [
{
    "topic" : "vscp/{{guid}}/topic/json",
    "qos" : 0,
    "retain" : false,
    "format": "json"
},
{
    "topic" : "vscp/{{guid}}/topic/xml",
    "qos" : 0,
    "retain" : false,
    "format": "xml"
},
{
    "topic" : "vscp/{{guid}}/topic/string",
    "qos" : 0,
    "retain" : false,
    "format": "string"
},
{
    "topic" : "vscp/{{guid}}/topic/binary",
    "qos" : 0,
    "retain" : false,
    "format": "binary"
},
{
    "topic" : "vscp/{{guid}}/topic/{{datetime}}/{{user}}/C",
    "qos" : 0,
    "retain" : false,
    "format": "json"
}    
],  
```

### topic :id=config-mqtt-publish-topic
This is the topic to publish event payloads to. It can be escaped for dynamic insertion of values. See [server publishing](./publishing_server.md) for more information.

### qos :id=config-mqtt-publish-qos
Quality of service for event payloads published on the topic. 

### retain :id=config-mqtt-publish-retain
Set to true to enable retain for event payload publishing..

### format :id=config-mqtt-publish-format

| Format | Description |
| ------ | ----------- |
| json | JSON formatted payload will be used. |
| xml  | XML formatted payload will be used. |
| string | Comma separated string formatted payload will be used. |
| binary | Binary formatted payload will be used. |



----
##  Level I Drivers :id=config-level1-driver

Define a VSCP daemon level I driver. If enabled the driver will be loaded. 

Level I drivers was in the past called CANAL (CAN Abstraction Layer) drivers (just because they are).

### Assigning GUID's to the interfaces of a VSCP daemon

Be sure to read [this part](#think-before-guid) before setting driver GUID's.

You are free to pick any GUID you like as long as it (normally) have the two least significant bytes of the GUID available for nodes on the interface to be used as there nicknames. Best and simplest is to select GUID's based on the machines IP or MAC address. For ip-address this is

IP:  FF:FF:FF:FF:FF:FF:FF:FD:YY:YY:YY:YY:XX:XX:XX:XX

YY... is the IP address. 

and for MAC address it is

IP:  FF:FF:FF:FF:FF:FF:FF:FE:YY:YY:YY:YY:XX:XX:XX:XX

YY... is the IP address. 

More information about GUID's can be found in the [specification](https://grodansparadis.github.io/vscp-doc-spec/#/./vscp_globally_unique_identifiers).

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
