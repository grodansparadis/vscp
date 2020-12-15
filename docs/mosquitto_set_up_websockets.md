# Manually install and build mosquitto with websockets

Current debian packages of the mosquitto broker appear to have websocket enabled so 

```bash
sudo apt install mosquitto
sudo apt install mosquitto-clients
```

may be all you may need. Skip the build step below if this is true on your machine.

This is a guide how to build and install mosquitto manually on Ubuntu with websockets enabled.

It is more or less the same as explained in the article ["Six Steps to install mosquitto 1.4.2 with websockets on debian wheezy"](http://www.xappsoftware.com/wordpress/2015/05/18/six-steps-to-install-mosquitto-1-4-2-with-websockets-on-debian-wheezy/comment-page-1/)

## Build

Exceptions:

1. mosquitto version 2.0.2 is used.
2. libwebsockets-dev was installed using apt-get instead of building from source.

### Install the dependencies

```
$ sudo apt-get update
$ sudo apt-get install build-essential python quilt python-setuptools python3
$ sudo apt-get install libssl-dev
$ sudo apt-get install cmake
$ sudo apt-get install libc-ares-dev
$ sudo apt-get install uuid-dev
$ sudo apt-get install daemon
$ sudo apt-get install libwebsockets-dev
```

### Download mosquitto 

```
$ cd Downloads/
$ wget http://mosquitto.org/files/source/mosquitto-2.0.2.tar.gz
$ tar zxvf mosquitto-2.0.2.tar.gz
$ cd mosquitto-2.0.2/
$ sudo nano config.mk
```

### Edit config.mk 

```
WITH_WEBSOCKETS:=yes
```

### Build mosquitto

```
$ make
$ sudo make install
$ sudo cp mosquitto.conf /etc/mosquitto
```

### Configure ports for mosquitto

Add the following lines to /etc/mosquitto/mosquitto.conf in the "Default Listener" section: 

```
port 1883
listener 9001
protocol websockets
```

### Add user for vscp 

```
$ sudo adduser vscp
```

### Reboot your computer

```bash
$ reboot
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



[filename](./bottom_copyright.md ':include')