# Setup the mosquitto MQTT broker

Setting up the MQTT broker is easy. Just do

```bash
sudo apt install mosquitto
sudo apt install mosquitto-clients
```

If you don't want the clients just skip them. But they can be good to have for debugging.

# Enable the mosquitto broker to run as a service

Issue

```
sudo systemctl enable mosquitto
sudo systemctl start mosquitto
```

to enable and start.

# Enable websockets

Add the following lines to the /etc/mosquitto/mosquitto.conf file to enable websockets


```
port 1883
listener 9001
protocol websockets
```

### Add a user

```
$ sudo adduser vscp
```

You can of course chose another username of your choice.

### Restart the mosquitto broker

```bash
$ sudo systemctl restart mosquitto
```

### Run mosquitto

```
$ mosquitto -c /etc/mosquitto/mosquitto.conf
```

## Configure

The official Mosquitto configuration document is [here](https://mosquitto.org/man/mosquitto-conf-5.html). It tells all really so look at it.

In order for changes in the mosquitto.conf file to become effective you must restart the mosquitto broker.

On windows you find it in the folder _c:\\mosquitto\\_ and on Linux you find it in _/etc/mosquitto_

You have a good walktrough [here](http://www.steves-internet-guide.com/mossquitto-conf-file/) which I can recommend.

## Securing

[Here](http://www.steves-internet-guide.com/mosquitto-tls/) is a good guid on how to setup a secure connection to a mosquitto broker.

## Bridging

TYou can bridge several brokers. [Here](Using Two MQTT Brokers with Broker Bridging) is a good document on this subject. There is also info on this subject [here](http://www.steves-internet-guide.com/mosquitto-bridge-configuration/).
