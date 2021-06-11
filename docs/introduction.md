# Introduction to the VSCP Daemon Software

The VSCP daemon is an interface between low level devices and the higher end device world. It can also connect to other VSCP servers and hardware nodes forming large and complex systems. The low end devices may be VSCP enabled or other types of devices thant know nothing about VSCP. The driver system of VSCP serve as a hardware abstraction layer that makes it possible to connect to anything that is out there. 

Two sorts of drivers are available, **Level I** and **Level II**, with slightly different capabilities. It is easy to construct new drivers and there are plenty of sample code on how to do so available.

From version 15.0 The VSCP daemon work tightly together with a local or remote MQTT broker. It can be looked on as a low lever interface for the MQTT and higher end world. 

Functionality that previously was incorporated in the VSCP daemon is now available as drivers. So now the built in web-server with websocket interfaces etc is instead a driver. Also the tcp/ip interface is available as a driver.

## Overview

![](./images/vscpd_connection_overview.png)

In the picture above a setup is shown from VSCP's point of view. The VSCP daemon is shown subscribing/publishing to a specific MQTT broker. In reality this can be several MQTT brokers and one or several arbitrary subscriptions/publishing topics to each.

A bridge to another MQTT broker that connects two of them together is also shown. Bridging is a good way to secure data into specific physical/logical domains. There can also be any number of bridges.

Now imagine a low end node connected to the level I interface of the VSCP daemon. If it delivers a temperature event this event can pass through the VSCP daemon and get published to the MQTT broker. A widget on an internal web page widget can subscribe to events of this type and show the current temperature in it's presentation interface.

The other way around is when a web widget user push a button and a VSCP event is sent over websocket to the broker and the topic it is sent over is subscribed to by a level II driver which light up a Bluetooth lamp in the bedroom when it detects the event.

Note that the abstraction of a temperature and a button pressed is preserved all the way from the user interface to the low level driver. Its is thus very easy to replace the Bluetooth lamp with for example a Zigbee lamp.

Only websocket connection is shown to the MQTT broker but of course higher level clients can subscribe/publish from/to it at whish. The Bluetooth/Zigbee lamp could be connected here equally well as behind the VSCP daemon and a driver. Where a node fits best depends on the the setup and what one think is best.

## Drivers

![](./images/vscpd_drv_structure.png)

Both level I drivers and level II drivers each define a seperate connection to a MQTT broker. Each can subscribe to VSCP events from any number of topics and publish VSCP events to any number of topics. It is thus easy to set up systems that work as a event hub and at the same time log and debug data and possibly do many other things with events.

## Why the new setup?

MQTT have so much code and infrastructure in place that it is impossible to ignore. It would also be very stupid not to use the good things that are avilable. Better to work together and reuse work of others efforts.

MQTT is not a IoT protocol even if many wrongly state that it is. It is however a IoT **transport** protocol. A job it does very well. 

VSCP on the other hand is a IoT protocol. Yes even a framework with all tools available. The combination MQTT + VSCP is just good match.  By combining them a lot of work can be reused. Important for a small project like VSCP. 

Take for example the VSCP tcp/ip interface. It does it's job well and is still (abd will be) valid for many VSCP devices. But MQTT can do the same thing also in a nice way. Adding security and tools and a lot more.

Another example is the VSCP [websockets ws1 and ws2 protocols](https://grodansparadis.github.io/vscp-doc-spec/#/./vscp_websocket). Both are constructed mainly as interfaces between sensors and user interface components that needs to be kept updated. MQTT also have a websocket interface defined and it can in a nice way do the same job as the VSCP varianst.

Also there is now a focus on the lower end of things in the VSCP development. Others projects do higher level stuff better. [Node-red](https://nodered.org/) for example is a good project for this such of things and there are plenty of others. For node-red there are two VSCP tools available. Checkout [node-red-contrib-vscp](https://flows.nodered.org/node/node-red-contrib-vscp) and [node-red-contrib-vscp-tcp](https://flows.nodered.org/node/node-red-contrib-vscp-tcp).

[filename](./bottom_copyright.md ':include')
