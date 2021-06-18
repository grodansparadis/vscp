# High end server/service probe

[CLASS1.PROTOCOL, Type=27, High end server/service probe](http://docs.vscp.org/spec/latest/#/./class1.protocol?id=type27-0x1b-high-end-serverservice-probe) can be sent on a segment to discover available VSCP services on that segment. The response from a node that have a VSCP server/service is [CLASS1.PROTOCOL, Type=28, High end server/service response](http://docs.vscp.org/spec/latest/#/./class1.protocol?id=type28-0x1c-high-end-serverservice-response) and for a Level II node [High end server/service capabilities](http://docs.vscp.org/spec/latest/#/./class2.protocol?id=type20-0x14-high-end-serverservice-capabilities). 

**Note** that a probe can result in none, one or many responses also from a single node on a segment.

A node that want to know what servers/services is available on a segment has two options. Either it listens on the [multicast announcement channel](./multicast_protocol_description_announce.md) for heartbeats and capabilities events over a minute or send a probe **CLASS1.PROTOCOL, Type=26**, High end server/service probe on the segment to immediately discover servers/services.

Note that [Who is there](http://docs.vscp.org/spec/latest/#/./class1.protocol?id=type31-0x1f-who-is-there) event have different response events for Level I and Level II nodes. A level I node respond with [CLASS1.PROTOCOL, Type=32, Who is there response](http://docs.vscp.org/spec/latest/#/./class1.protocol?id=type32-0x20-who-is-there-response). A level II node respond with [CLASS2.PROTOCOL, Type=32 Level II who is response](http://docs.vscp.org/spec/latest/#/./class2.protocol?id=type32-0x20-level-ii-who-is-there-response) to this event.



[filename](./bottom_copyright.md ':include')
