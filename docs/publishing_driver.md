# Driver publishing

Each driver connect to a MQTT broker of choice and can have an arbitrary number of topics which to it will be publishing events to. Topics can be escaped for dynamic data, qos (quality of service) as well as retain can be set induvidully. Payload publishing format for the VSCP events can induvidually be set json (default), XML, string and binary for each topic.

See the [server publishing page](publishing_server.md) for more informantion. Driver publish event payloads with the same mechanism.

If you want all drivers or groups of drivers talk to each other just publish/subscribe to a common topic.

```json
"bescape-pub-topics": true,
  "user-escapes": {
      "escape1": "valu1",
      "escape2": "valu2"
  },
  "publish": [
      {
          "topic" : "publish/topic/json",
          "qos" : 0,
          "retain" : false,
          "format": "json"
      },
      {
          "topic" : "publish/topic/xml",
          "qos" : 0,
          "retain" : false,
          "format": "xml"
      },
      {
          "topic" : "publish/topic/string",
          "qos" : 0,
          "retain" : false,
          "format": "string"
      },
      {
          "topic" : "publish/topic/binary",
          "qos" : 0,
          "retain" : false,
          "format": "binary"
      },
      {
          "topic" : "publish/topic/{{datetime}}/{{user}}/C",
          "qos" : 0,
          "retain" : false,
          "format": "json"
      }    
  ],
```

## Topic format :id=driver-publish-topic-format
The MQTT topic often with escapes. See [server subscribes](subscribe_server.md) for a full description.

## Topic qos :id=driver-publish-topic-qos
Quality of service for VSCP event payloads published on this topic.

## Topic retain :id=driver-publish-topic-retain
Set tto *true* to enable retain.

## Topic format :id=driver-publish-topic-format
One are free to setup topic content in any way one want. With a bit of thinking it is possible to achieve very nice results. 

One method one can stick to is to set up topics like this

```
vscp/{{guid}}/{{class}}/{{type}}/{{nickname}}
```

This give many possibilities when subscribing to VSCP events.

### Subscribe to vscp/FF:FF:FF:FF:FF:FF:FF:FE:60:A4:4C:E7:76:5A:00:01/#

All events from the node defined by the particular GUID (FF:FF:FF:FF:FF:FF:FF:FE:60:A4:4C:E7:76:5A:00:01) set here will be received.

### Subscribe to vscp/FF:FF:FF:FF:FF:FF:FF:FE:60:A4:4C:E7:76:5A:00:01/10/#

All [measurement events](https://grodansparadis.github.io/vscp-doc-spec/#/./class1.measurement) sent by the node with this specific GUID will be received.

### Subscribe to vscp/FF:FF:FF:FF:FF:FF:FF:FE:60:A4:4C:E7:76:5A:00:01/10/6/#

All [temperature measurement events](https://grodansparadis.github.io/vscp-doc-spec/#/./class1.measurement?id=type6) sent by the node with this specific GUID will be received.

### Subscribe to vscp/+/10/6/#

All [temperature measurement events](https://grodansparadis.github.io/vscp-doc-spec/#/./class1.measurement?id=type6) sent to all nodes will be received.

