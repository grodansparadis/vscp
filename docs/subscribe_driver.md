# Driver subscriptions

A driver can subscribe to any number of topics. At least as long as the broker and the computer that runs the VSCP daemon can handle it.

Driver subscriptions work in the same way as [server subscriptions](./subscribe_server.md) so read the information in that document as well.

Setting looks like this

```json
"subscribe": [
  {
      "topic": "test1/topic/A",
      "qos": 0,
      "v5-options": 0,
      "format": "auto"
  },
  {
      "topic": "test2/topic/B",
      "qos": 0,
      "v5-options": 0,
      "format": "auto"
  },
  {
      "topic": "test/#",
      "qos": 0,
      "v5-options": 0,
      "format": "auto"
  },
  {
      "topic": "test2/#",
      "qos": 0,
      "v5-options": 0,
      "format": "auto"
  }
],
```

## topics :id=subscribe-driver-qos

A driver can subscribe to any topic.  One method that is recommended is to set up publish topics like this

```
vscp/{{guid}}/{{class}}/{{type}}/{{nickname}}
```

This give many possibilities when subscribing to VSCP events.

### Subscribe to _vscp/FF:FF:FF:FF:FF:FF:FF:FE:60:A4:4C:E7:76:5A:00:01/#_

All events from the node defined by the particular GUID (FF:FF:FF:FF:FF:FF:FF:FE:60:A4:4C:E7:76:5A:00:01) set here will be received.

### Subscribe to _vscp/FF:FF:FF:FF:FF:FF:FF:FE:60:A4:4C:E7:76:5A:00:01/10/#_

All [measurement events](https://grodansparadis.github.io/vscp-doc-spec/#/./class1.measurement) sent by the node with this specific GUID will be received.

### Subscribe to _vscp/FF:FF:FF:FF:FF:FF:FF:FE:60:A4:4C:E7:76:5A:00:01/10/6/#_

All [temperature measurement events](https://grodansparadis.github.io/vscp-doc-spec/#/./class1.measurement?id=type6) sent by the node with this specific GUID will be received.

### Subscribe to _vscp/+/10/6/#_
All [temperature measurement events](https://grodansparadis.github.io/vscp-doc-spec/#/./class1.measurement?id=type6) sent to all nodes will be received.


## qos :id=subscribe-driver-qos
Quality of service for the subscription (0,1,2).

## v5-options :id=subscribe-driver-v4-options
MQTT version 5 options for the subscription.

| Option | Description |
| ------ | ----------- |
| MQTT_SUB_OPT_NO_LOCAL	| with this option set, if this client publishes to a topic to which it is subscribed, the broker will not publish the message back to the client. |
| MQTT_SUB_OPT_RETAIN_AS_PUBLISHED | with this option set, messages published for this subscription will keep the retain flag as was set by the publishing client.  The default behavior without this option set has the retain flag indicating whether a message is fresh/stale. |
| MQTT_SUB_OPT_SEND_RETAIN_ALWAYS	| with this option set, pre-existing retained messages are sent as soon as the subscription is made, even if the subscription already exists.  This is the default behavior, so it is not necessary to set this option. |
| MQTT_SUB_OPT_SEND_RETAIN_NEW |with this option set, pre-existing retained messages for this subscription will be sent when the subscription is made, but only if the subscription does not already exist. |
| MQTT_SUB_OPT_SEND_RETAIN_NEVER |	with this option set, pre-existing retained messages will never be sent for this subscription. |

## format :id=subscribe-driver-format
The format the MQTT payload is expected to carry the VSCP event in. 

Valid values are

| Format | Description |
| ------ | ----------- |
| auto | The VSCP daemon will try to detect the payload format. |
| json | JSON formatted payload will be expected. |
| xml  | XML formatted paylaod will be expected. |
| string | Comma separated string formatted payload will be expected. |
| binary | Binary formatted payload will be expected. |

The value set in [subscribe-format](#config-mqtt-general-subscribe-format) will be used if not set.