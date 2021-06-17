# Server publishing

The VSCP daemon publish events from several points. First the daemon itself publish events. and then each driver connected to the VSCP daemon publish events.

The VSCP daemon publish start up information, information about drivers and heart beat and information events.

### Topics

Default topics for the daemon looks like this

```json
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
}
```

That is four topics each with different payload coding (json,xml,string,binary). 

The {{guid}} part is replaced with the configured GUID set for the daemon (Setting **"guid"** in the configuration file.)

Topics with escapes like {{guid}} are created on the fly so a topic string on the form

```json
{
    "topic" : "vscp/{{guid}}/topic/{{datetime}}/{{user}}/C",
    "qos" : 0,
    "retain" : false,
    "format": "json"
}
```

will create a new topic for each published event as {{datetime}} and oir the other escapes changes.

One can also use VSCP class and type information in events like this

```json
{
    "topic" : "vscp/{{guid}}/{{class}}/{{type}}/{{nickname}}",
    "qos" : 0,
    "retain" : false,
    "format": "json"
}
```

actually the preferred coding for events as it allows for effective filtering.

Subscribing topics like the following explains this

#### **vscp/"some guid"/#**
Will get all events from a specific device.

#### vscp/"some guid"/"some class"/#
Will get events of a specific class from a specific device.

#### vscp/+/"some class"/#  				
Will get events of a specific class from a all devices.

#### vscp/"some guid"/"some class"/"some type"/#
Will get events of a specific class and a specif type from a specific device.

#### vscp/+/"some class"/"some type"/#
Will get events of a specific class and a specif type from all devices.

It is also possible to create topics with guid groups. For example publish the following escape sequence on a topic part

```mustash
vscp/{{xguid[15]}}:xguid[14]}}:{{xguid[13]}}:{{xguid[12]}}:{{xguid[11]}}:{{xguid[10]}}:{{xguid[9]}}:{{xguid[8]}}/{{class}}/{{type}}/
```

and subscribing to 

```bash
vscp/FF:FF:FF:FF:FF:FF:FF:FD/0/10
```

will get all temperature events with a [GUID based on an ipv4 address](https://grodansparadis.github.io/vscp-doc-spec/#/./vscp_globally_unique_identifiers).

to create a string consisting of the eight most significant guid's. 

There is a good description [here](https://www.hivemq.com/blog/mqtt-essentials-part-5-mqtt-topics-best-practices/)

### Topic escapes

Valid escapes are

| Escape | Description |
| ------ | ----------- |
| {{guid}} | GUID for the event |
| {{ifguid}} | Interface GUID for the daemon or the driver |
| {{ifguid[0..15]}} | Specified byte of interface GUID 0-15 for the event in decimal |
| {{xifguid[0..15]}} | Specified byte of interface GUID 0-15 for the event in hexadecimal |
| {{guid.msb}} | Most significant byte of GUID for the event in decimal |
| {{xguid.msb}} | Most significant byte of GUID for the event in hexadecimal |
| {{guid.lsb}} | Least significant byte of GUID for the event in decimal |
| {{xguid.lsb}} | Least significant byte of GUID for the event in hexadecimal |
| {{guid[0..15]}} | Specified byte of GUID 0-15 for the event in decimal |
| {{xguid[0..15]}} | Specified byte of GUID 0-15 for the event in hexadecimal |
| {{data[0..511]}} | Specified byte of data 0-511 in decimal |
| {{xdata[0..511]}} | Specified byte of data 0-511 for the event in hexadecimal |
| {{sizedata}} | Data size for the event in decimal. |
| {{xsizedata}} | Data size for the event in hexadecimal. |
| {{nickname}} | Nickname for the event in decimal. The nickname is formed of the two least significant bytes of the GUID (GUID[14]*256 + GUID[15]), with the MSB in byte GUID[14]. |
| {{xnickname}} | Nickname for the event in hexadecimal. The nickname is formed of the two least significant bytes of the GUID (GUID[14]*256 + GUID[15]), with the MSB in byte GUID[14]. |
| {{class}} | VSCP class for the event in decimal. |
| {{xclass}} | VSCP class for the event in hexadecimal. |
| {{type}} | VSCP type for the event in decimal. |
| {{xtype}} | VSCP type for the event in hexadecimal. |
| {{class-token}} | VSCP class token for the event (ex. CLASS1.PROTOCOL). See the [VSCP specification](https://grodansparadis.github.io/vscp-doc-spec/#/) or the lists defined [here](https://vscp.org/events/) for available tokens.  |
| {{type-token}} | VSCP type token for the event (ex. VSCP_TYPE_PROTOCOL_SET_NICKNAME). See the [VSCP specification](https://grodansparadis.github.io/vscp-doc-spec/#/) or the lists defined [here](https://vscp.org/events/) for available tokens. |
| {{head}} | Head for the event in decimal. |
| {{xhead}} | Head for the event in hexadecimal. |
| {{obid}} | Obid for the event in decimal. |
| {{xobid}} | Obid for the event in hexadecimal. |
| {{datetime}} | UTC datetime for the event. (ex. 2021-06-17T07:22:55) |
| {{year}} | UTC datetime year for the event in decimal. |
| {{month}} | UTC datetime month for the event in decimal. |
| {{day}} | UTC datetime day for the event in decimal. |
| {{hour}} | UTC datetime hour for the event in decimal. |
| {{minute}} | UTC datetime minute for the event in decimal. |
| {{second}} | UTC datetime second for the event in decimal. |
| {{local-datetime}} | Local datetime for the event. (ex. 2021-06-17T09:22:55) |
| {{local-year}} | Local datetime year for the event in decimal. |
| {{local-month}} | Local datetime month for the event in decimal. |
| {{local-day}} | Local datetime day for the event in decimal. |
| {{local-hour}} | Local datetime hour for the event in decimal. |
| {{local-minute}} | Local datetime minute for the event in decimal. |
| {{local-second}} | Local datetime second for the event in decimal. |
| {{cientid}} | Clientid for the current published topic. |
| {{user}} | Username for user from settings. |
| {{host}} | Host from settings. |
| {{fmtpublish}} | Publishing format as a string "json", "xml", "string", "binary". |


In addition to the above user escapes can be configured. This is done with the "user-escapes" tag in the configuration file. Defining escapes like the following

```json
"user-escapes": {
    "escape1": "value1",
    "escape2": "value2"
}
```

will when {{escape1}} is inserted in the topic translate to "value1" and {{escape2}} is inserted in the topic translate to "value2". Only values on string form is accepted.

Note that "bescape-pub-topics" tag must be set to true.

```json
"bescape-pub-topics": true,
```

## Driver publishing

You can set up as many topics as you want that events will be published on for a driver. All topics accept escapes to add dynamic information and also valid information such as qos and retain can be set for each topic individually.

Escape are the same as [escapes]("./publishing.md#Topic escapes") for server publishing above.



