# Setting up the System

The VSCP Daemon is easy to setup. Just install the server and then configure the drivers you want to use. It is recommended to set up a MQTT broker locally also to get the utilize the full power of the system.

## MQTT

From this version MQTT is central and the vscp server **must have** a MQTT broker to function. Initial configurations files use the Mosquitto test server (test.mosquitto.org:1883) which is open and free to use for testing. There are other [free and open brokers](https://mntolia.com/10-free-public-private-mqtt-brokers-for-testing-prototyping/) available for test. In a full production environment you should almost always setup your own broker or subscribe to some of the brokers that are available (such as the ones in the lin above).

To test, debug and do work with the vscp daemon server you also need a MQTT client. We use the [VSCP Works tool](https://github.com/grodansparadis/vscp-works-qt) for much of our work. This tool is currently being rewritten and MQTT support and other functionality is added as time goes.

For development we use the Mosquitto client tools. The main tools we use are [mosquitto_pub](https://mosquitto.org/man/mosquitto_pub-1.html) and [mosquitto_sub](https://mosquitto.org/man/mosquitto_sub-1.html). On a debian based system you can install them with

```bash
sudp apt install mosquitto-clients
```

Most samples in this documentation referee to these tools in some way.

Another MQTT tool that is useful to have installed is the [MQTT Explore](http://mqtt-explorer.com/).


[filename](./bottom_copyright.md ':include')
