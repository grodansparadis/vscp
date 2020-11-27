# Install mosquitto with websockets

This is a guide how to install mosquitto on Ubuntu with websockets enabled.

It is more or less the same as explained in the article "Six Steps to install mosquitto 1.4.2 with websockets on debian whezy"
http://www.xappsoftware.com/wordpress/2015/05/18/six-steps-to-install-mosquitto-1-4-2-with-websockets-on-debian-wheezy/comment-page-1/

Exceptions:

1. mosquitto version 1.4.10 is used.
2. libwebsockets-dev was installed using apt-get instead of building from source.

## Install the dependencies

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

## Download mosquitto 

```
$ cd Downloads/
$ wget http://mosquitto.org/files/source/mosquitto-1.4.10.tar.gz
$ tar zxvf mosquitto-1.4.10.tar.gz
$ cd mosquitto-1.4.10/
$ sudo nano config.mk
```

## Edit config.mk 

```
WITH_WEBSOCKETS:=yes
```

## Build mosquitto

```
$ make
$ sudo make install
$ sudo cp mosquitto.conf /etc/mosquitto
```

## Configure ports for mosquitto

Add the following lines to /etc/mosquitto/mosquitto.conf in the "Default Listener" section: 

```
port 1883
listener 9001
protocol websockets
```

## Add user for mosquitto 

```
$ sudo adduser mosquitto
```

## Reeboot computer

```
$ reboot
```

## Run mosquitto

```
$ mosquitto -c /etc/mosquitto/mosquitto.conf
```