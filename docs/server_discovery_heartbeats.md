# Hearbeats and announcements

A level I node sends [CLASS1.INFORMATION, Type=9, node heartbeats](http://docs.vscp.org/spec/latest/#/./class1.information?id=type9-0x09-node-heartbeat) on the segment it is connected to. muticast announce channel.  

A level II node sends [CLASS2.INFORMATION, Type=2, Level II Node Heartbeat](http://docs.vscp.org/spec/latest/#/./class2.information?id=type2-0x02-level-ii-node-heartbeat)

A level II node in addition sends [CLASS2.PROTOCOL, Type=20, High end server/service capabilities](http://docs.vscp.org/spec/latest/#/./class2.protocol?id=type20-0x14-high-end-serverservice-capabilities) on the multicast announce channel.

A level II node, like the VSCP server, that act as a gateway for other nodes send [CLASS2.Information, Type=3, Level II Proxy Node Heartbeat](http://docs.vscp.org/spec/latest/#/./class2.information?id=type3-0x03-level-ii-proxy-node-heartbeat).

[filename](./bottom_copyright.md ':include')
