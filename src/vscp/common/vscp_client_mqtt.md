# vscp_client_mqtt 

This is one of the classes in the VSCP client collection. The collection defines programmatic functionality to send and receive VSCP events over different transports.

This class uses the [MQTT protocol](https://mqtt.org/) as its transport medium.

## Configuration

The method [initFromJson](#initFromJson) have a string as its argument. This string is on json format and define configuration functionality 

```json
{
      "bind" : "interface",
      "host" : "[s]tcp://192.168.1.7:1883",
      "port" : 1883,
      "mqtt-options" : {
        "tcp-nodelay" : true,
        "protocol-version": 500,
        "receive-maximum": 20,
        "send-maximum": 20,
        "ssl-ctx-with-defaults": 0,
        "tls-ocsp-required": 0,
        "tls-use-os-certs" : 0
      },
      "user" : "vscp",
      "password": "secret",
      "clientid" : "mosq-vscp-daemon-000001",
      "publish-format" : "json",
      "subscribe-format" : "auto",
      "qos" : 1,
      "bcleansession" : false,
      "bretain" : false,
      "keepalive" : 60,
      "bjsonmeasurementblock" : true,
      "reconnect" : {
        "delay" : 2,
        "delay-max" : 10,
        "exponential-backoff" : false
      },
      "filter": {
        "priority-filter": 0,
        "priority-mask": 0,
        "class-filter": 0,
        "class-mask": 0,
        "type-filter": 0,
        "type-mask": 0,
        "guid-filter": "00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00",
        "guid-mask": "00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00"
      },
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
      "will": {
          "topic": "topic",
          "qos": 0,
          "retain": true,
          "payload": "string/json/xml"
      },
      "subscribe": [
        {
          "topic": "subscribe/topic/A",
          "qos": 0,
          "v5-options": 4
        },
        {
          "topic":"subscribe/topic/B",
          "qos": 0,
          "v5-options": "NO_LOCAL, RETAIN_AS_PUBLISHED, SEND_RETAIN_ALWAYS, SEND_RETAIN_NEW, SEND_RETAIN_NEVER"
        }
      ],
      "bescape-pub-topics": true,
      "user-escapes": {
        "escape1": "valu1",
        "escape2": "valu2"
      },
      "publish": [
        {
          "topic": "publish/topic/A",
          "qos": 0,
          "bretain": false,
          "format": "json"
        },
        {
          "topic": "publish/topic/B",
          "qos": 0,
          "bretain": false,
          "format": "xml"
        }
      ],
      "v5" : {
          "user-properties": {
              "prop1" : "value",
              "prop2" : "value"
          }
      }
}
```

### bind
Optional. 
Set ip-address for interface to bind to.
Default: No binding to a specific interface.

### host 
```
[s]tcp\\addr:port
```
If prefix is **stcp:\\\\** a secure connection is requested. If prefixed with **tcp:\\\\** an unsecure connection i requested. Addresss can be ip-address or an address that needs o be DSN resolved. Port is optional and is set to default 1883 if it is.Port can be set on it's own also. See below

### port 
If host contains server port then a separate defenition of a port is not needed. If not set it here. If not set at all it is set to the default value 1883.

### mqtt-options

#### tcp-nodelay 
Set to true to disable Nagle’s algorithm on client sockets.  This has the effect of reducing latency of individual messages at the potential cost of increasing the number of packets being sent.  

Defaults is **true**, which means Nagle remains disabled.


#### protocol-version 
This is the requested MQTT version to work with. Valid values are

  - **310** for version 3.10
  - **311** for version 3.11
  - **500** for version 5

Default is **311**.

#### receive-maximum
Value can be set between 1 and 65535 inclusive, and represents the maximum number of incoming QoS 1 and QoS 2 messages that this client wants to process at once.  Defaults to 20.  This option is not valid for MQTT v3.1 or v3.1.1 clients.  Note that if the MQTT_PROP_RECEIVE_MAXIMUM property is in the proplist passed to mosquitto_connect_v5(), then that property will override this option.  Using this option is the recommended method however.

Default is **20**

#### send-maximum
Value can be set between 1 and 65535 inclusive, and represents the maximum number of outgoing QoS 1 and QoS 2 messages that this client will attempt to have “in flight” at once.  Defaults to 20.  This option is not valid for MQTT v3.1 or v3.1.1 clients.  Note that if the broker being connected to sends a MQTT_PROP_RECEIVE_MAXIMUM property that has a lower value than this option, then the broker provided value will be used.

Default is **20**

#### ssl-ctx-with-defaults
If value is set to a non zero value, then the user specified SSL_CTX passed in using MOSQ_OPT_SSL_CTX will have the default options applied to it.  This means that you only need to change the values that are relevant to you.  If you use this option then you must configure the TLS options as normal, i.e.  you should use mosquitto_tls_set to configure the cafile/capath as a minimum.  This option is only available for openssl 1.1.0 and higher.

Default is **0**

#### tls-ocsp-required
Set whether OCSP checking on TLS connections is required.  Set to 1 to enable checking, or 0 (the default) for no checking.

Default is **0**

#### tls-use-os-certs
Set to 1 to instruct the client to load and trust OS provided CA certificates for use with TLS connections.  Set to 0 (the default) to only use manually specified CA certs.

Default is **0**

### user
Username as a string for the mosquitto instance. By default, no username or password will be sent.

### password
Password as a string for the mosquitto instance. By default, no username or password will be sent.

### clientid" : "mosq-vscp-daemon-000001",
A string to use as the client id for the mosquitto instance.  If not set, a random client id will be generated. If not set **bcleansession** must be true.

Default is not set.

### bcleansession
Set to true to instruct the broker to clean all messages and subscriptions on disconnect, false to instruct it to keep them.  Note that a client will never discard its own outgoing messages on disconnect. Must be set to true if the client id parameter is not set.

Default is **false**

### bjsonmeasurementblock

If enabled and the event is a measurement and the publish format is in json, setting this to true will add a measurement block to the json data like this

```json
"measurement" : {
  "value" : 1.23,
  "unit" : 0,
  "sensorindex" : 1,
  "zone" : 11,
  "subzone" : 22
}
```

**Important!** One should never expect this block of data to be present for a measurement. It is better for a client to use the standard conversion functions for measurement content.

default is **true**

### publish-format
Default payload publish format. If not specified in the publishing configuration this is the value that will be used. Possible settings are

 - **json** for JSON formatted data
 - **xml** for XML formatted payloads.
 - **string** for string (comma separated) payloads.
 - **binary** for binary payloads.

Default is **json**

### subscribe-format
Subscribe format. If set to **auto** json-, xml-, string- and binary formatted payloads will be accepter. If set to a specific format other then **auto** the payload need to be on that forat to be accepter.

 - **auto** All formats are accepted.
 - **json** for JSON formatted data
 - **xml** for XML formatted payloads.
 - **string** for string (comma separated) payloads.
 - **binary** for binary payloads.

Default is **auto**

### qos
Integer value 0, 1 or 2 indicating the Quality of Service to be used as default for the instance if not set in another way.

Default is **1**

### bretain
Set to true to make retained messages as as default for the instance if not set in another way.

Default is **false**

### keepalive
Set the number of seconds after which the broker should send a PING message to the client if no other messages have been exchanged in that time.

Default is **60**

### reconnect
Control the behaviour of the client when it has unexpectedly disconnected.  The default behaviour if this function is not used is to repeatedly attempt to reconnect with a delay of 1 second until the connection succeeds.

Use reconnect_delay parameter to change the delay between successive reconnection attempts.  You may also enable exponential backoff of the time between reconnections by setting **exponential-backoff** to true and set an upper bound on the delay with **delay-max**.

**Example 1**
delay=2, delay_max=10, exponential_backoff=False Delays would be: 2, 4, 6, 8, 10, 10, ...

**Example 2**
delay=3, delay_max=30, exponential_backoff=True Delays would be: 3, 6, 12, 24, 30, 30, ...

#### delay
Set the number of seconds to wait between reconnects.

Default is **2** seconds

#### delay-max
Set the maximum number of seconds to wait between reconnects.

Default is **10** tries

#### exponential-backoff
use exponential backoff between reconnect attempts.  Set to true to enable exponential backoff.

Default is **false**

### filter
Set VSCP filter for incoming events.

#### priority-filter
Filter for VSCP priority (0-7).

Default is **0**

#### priority-mask
Mask for VSCP priority (0-7).

Defautl is **0**

#### class-filter
Filter for VSCP class (0-65525).

Default **0**

#### class-mask
Mask for VSCP class (0-65525).

Default is **0**

#### type-filter
Filter for VSCP type (0-65525).

Default is **0**

#### type-mask
Mask for VSCP type (0-65525).

Default is **0**

#### guid-filter
Filter for VSCP GUID.

Default is **"00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00"**

#### guid-mask
Mask for VSCP GUID.

Default is **"00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00"**
  
### tls
Configure the client for certificate based SSL/TLS support.

Cannot be used in conjunction with mosquitto_tls_psk_set.

Define the Certificate Authority certificates to be trusted (ie. the server certificate must be signed with one of these certificates) using cafile.

If the server you are connecting to requires clients to provide a certificate, define certfile and keyfile with your client certificate and private key.  If your private key is encrypted, provide a password callback function or you will have to enter the password at the command line.

#### insecure 
Configure verification of the server hostname in the server certificate.  If value is set to true, it is impossible to guarantee that the host you are connecting to is not impersonating your server.  This can be useful in initial server testing, but makes it possible for a malicious third party to impersonate your server through DNS spoofing, for example.  Do not use this function in a real system.  Setting value to true makes the connection encryption pointless.

if set to false, the default, certificate hostname checking is performed.  If set to true, no hostname checking is performed and the connection is insecure.

Default is **false**

#### cafile
Set path to a file containing the PEM encoded trusted CA certificate files.  Either cafile or capath must not be empty.

Default is **empty**

#### capath
Set path to a directory containing the PEM encoded trusted CA certificate files. Either cafile or capath must not be empty.

Default is **empty**

#### certfile
Set path to a file containing the PEM encoded certificate file for this client.  If empty, keyfile must also be empty and no client certificate will be used.

Default is **empty**

#### keyfile
Set path to a file containing the PEM encoded private key for this client.  If empty, certfile must also be empty and no client certificate will be used.

Default is **empty**

#### pwkeyfile
If keyfile is encrypted, set pw_callback to allow your client to pass the correct password for decryption.  If set to NULL, the password must be entered on the command line.  Your callback must write the password into “buf”, which is “size” bytes long.  The return value must be the length of the password.  “userdata” will be set to the calling mosquitto instance.  The mosquitto userdata member variable can be retrieved using mosquitto_userdata.

Default is **empty**

#### no-hostname-checking


Default is **true**

#### cert-reqs
If set to 0 the server will not be verified in any way.
If set to 1 the server certificate will be verified and the connection aborted if the verification fails.  The default and recommended value is to set parameter to 1.  Using a value of 0 provides no security.

Default is **0**

#### version
Set the version of the SSL/TLS protocol to use as a string.  If not defined, the default value is used.  The default value and the available values depend on the version of openssl that the library was compiled against.  For openssl >= 1.0.1, the available options are _"tlsv1.2"_, _"tlsv1.1"_ and _"tlsv1"_, with _"tlv1.2"_ as the default.  For openssl < 1.0.1, only _"tlsv1"_ is available.

#### ciphers
Set a string describing the ciphers available for use.  See the “openssl ciphers” tool for more information.  If not defined, the default ciphers will be used.

#### psk
Configure the client for pre-shared-key based TLS support. If used _cafile_, _capath_, _certfile_ and _keyfile_ should be empty.

Set the pre-shared-key in hex format with no leading “0x”.

#### psk-identity
Set the identity of this client.  May be used as the username depending on the server settings.

### will

Define how mosquitto last will is handled. Configure will information for a mosquitto instance, with attached properties.  By default, clients do not have a will.  

If the mosquitto instance `mosq` is using MQTT v5, the `properties` will be applied to the Will.  For MQTT v3.1.1 and below, the `properties` argument will be ignored.

#### topic
Set the topic on which to publish the will.

#### qos
Integer value 0, 1 or 2 indicating the Quality of Service to be used for the will.

#### retain
Set to true to make the will a retained message.

#### payload
Defines the last will payload as a string. Valid string lengths are between 0 and 268,435,455.

### subscribe
Here one or many subscriptions are defined. A typical two subscription definition looks like this

```json
"subscribe": [
  {
    "topic": "subscribe/topic/A",
    "qos": 0,
    "v5-options": 4,
  },
  {
    "topic":"subscribe/topic/B",
    "qos": 0,
    "v5-options": "NO_LOCAL, RETAIN_AS_PUBLISHED, SEND_RETAIN_ALWAYS, SEND_RETAIN_NEW, SEND_RETAIN_NEVER".
  }
],
```

#### topic
Ths topic for the subscription. Allow for wildcards.

#### qos": 0,
Integer value 0, 1 or 2 indicating the Quality of Service to be used for the subscription.

Default is **0**

#### v5-options
MQTT v5 subscribe options. Only valid if the instance is v5 enabled. The options can either be set as a number or as a string with tokens

 - NO_LOCAL - with this option set, if this client publishes to a topic to which it is subscribed, the broker will not publish the message back to the client.
 - RETAIN_AS_PUBLISHED - with this option set, messages published for this subscription will keep the retain flag as was set by the publishing client.  The default behaviour without this option set has the retain flag indicating whether a message is fresh/stale.
 - SEND_RETAIN_ALWAYS - with this option set, pre-existing retained messages are sent as soon as the subscription is made, even if the subscription already exists.  This is the default behaviour, so it is not necessary to set this option.
 - SEND_RETAIN_NEW - with this option set, pre-existing retained messages for this subscription will be sent when the subscription is made, but only if the subscription does not already exist.
 - SEND_RETAIN_NEVER - with this option set, pre-existing retained messages will never be sent for this subscription.

Default is **0**

### escape-pub-topics

If enabled escapes in topics on [mustach format](https://mustache.github.io/) will be substituted with current values.  It is also possible to set user defined eescapes 

Default is **true**

### user-escapes

User escapes are user defined substitutions (=escapes) that when user in topic with mustach syntax (_{{escape}}_) will get the escape toke replaced by the escape value. In a configuration file they are derfined as

```json
"user-escapes": {
  {"escape1":"valu1"},
  {"escape2":"valu2"},
}
```

but they can also be defined by a controling program by calling the method [setUserEscape](#setUserEscape)

default is **empty**.

### publish

Here are the topics incomming events are published to. You can defined one or many.

A typical definition for two defined publish destinations looks like this

```json
"publish": [
  {
    "topic": "publish/topic/A",
    "qos": 0,
    "bretain": false,
    "format": "json"
  },
  {
    "topic": "publish/topic/B",
    "qos": 2,
    "bretain": false,
    "format": "xml"
  }
],
```
        
#### topic
Topic as a string to publish to.

#### qos
Integer value 0, 1 or 2 indicating the Quality of Service to be used for the subscription.

Default is **0**

#### bretain
Set to true to make the published event a retained message.

Default is **false**

#### format
Set the format for the published event. Valid settings are

 - **json** for JSON formatted data
 - **xml** for XML formatted payloads.
 - **string** for string (comma separated) payloads.
 - **binary** for binary payloads.

Default is **json**
        
### v5

Version 5 specific settings

#### user-properties
Set v5 poperty pairs in an array where each pair consist of a prperty name and a value where both is on string form. 

## Methods

## initFromJson

```c++
bool vscpClientMqtt::initFromJson(const std::string &config)
```

This is a method parse a string on JSON format and read configuration from it. The configuration format is defined above in [Configuration](#Configuration)

**Parameters**

 * **config** Configuration string on JSON format.

**Returns**

 * **true** och **success**, false on failure.

## getConfigAsJson

Return the current configuration as a sting on JSON format.

```c++
std::string vscpClientMqtt::getConfigAsJson(void)
```

**Returns**

A string with configuration on JSON format or an empty string if an error occured or no configuration has been set.

