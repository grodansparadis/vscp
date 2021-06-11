# Manually install and build mosquitto with websockets

Current debian packages of the mosquitto broker appear to have websocket enabled so 

```bash
sudo apt install mosquitto
sudo apt install mosquitto-clients
```

may be all you may need. Skip the build step below if this is true on your machine.

----

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

see [this document](./mosquitto_set_up_broker.md) on how to configure the broker.




[filename](./bottom_copyright.md ':include')