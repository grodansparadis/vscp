# Setting up the System

The VSCP Daemon is easy to setup. Just install the server and then configure the drivers you want to use. It is recommended to set up a local MQTT broker also but one can use one of the [public MQTT brokers](https://mntolia.com/10-free-public-private-mqtt-brokers-for-testing-prototyping/) for testing.

## Setting up the system on Linux

To install the VSCP daemon on Linux/Unix you need to install the binary package or build the system from source. The build process is simple and it usually don't give any problems. You can find the latest binaries and source files [here](https://github.com/grodansparadis/vscp/releases)

### Installing from binary package (preferred install method)

The binary package is supplied as a Debian package. This should work as an installation method on all distribution's that can handle Debian packages. There are currently two variants available. 

   * **x86_64** should work on a Debian/Ubuntu or other Debian derived 64-bit system.
   * **arm7hf** should work on any arm7 system like Raspberry Pi.

You find the Debian package [here](https://github.com/grodansparadis/vscp/releases). Download the package for your architecture and install with

When this is written the 15.0.0 release is the latest and the Debian package is named **vscpd_15.0.0-1_amd64.deb** Installing this package with
 

```bash
    sudo apt install ./vscpd_15.0.0-1_amd64.deb
```

after downloading the file.

When you have installed the package you can enable and start the VSCP daemon with

```bash
systemctl enable vscpd
systemctl start vscpd
```

But the install script should normally do this for you automatically.

Thats it! You can start to work with VSCP.

Now read the section about configuring your system.

### Private Debian package repository - Experimental

**Note:** Use only for testing for now!!!!

The VSCP project also have a private package repository which currently is experimental. The repository holds files for amd64/i385/armhf that should work on all debian derived systems such as Ubuntu and Raspbian as well as on Debian itself. The repository holds all drivers, the VSCP daemon and many other packages.

To use it you must add a signing key to your apt install system. You do this with

```
curl -s http://apt.vscp.org/apt.vscp.org.gpg.key | sudo apt-key add -
```

then add (replace 'buster' with code name for your system (get code name with 'lsb_release -c'))

```
deb http://apt.vscp.org/debian buster main
```

to the file

```
/etc/apt/sources.list
```

After the setup you can install with

```
sudo apt update
sudo apt install vscpd
```

and upgrades will be handled for you.


### Building binaries from source

#### Step 1

Prepare the build environment with

```bash
sudo apt update
sudo apt install build-essential
sudo apt install git libssl-dev libsystemd-dev libexpat-dev libwrap0-dev libz-dev libmosquitto-dev
```    
to install the build tools and required libraries. 

#### Step 2

The source tar/zip files for vscpd is available at [GitHub](https://github.com/grodansparadis/vscp/releases). Download the latest version and unpack in the folder of your choice. .

Unpack the source in a folder of your choice with

```bash
tar -zxvf file
```

or

```bash
unzip file
```


If you want to use bleeding edge source use git and clone the repository with

```bash
git clone https://github.com/grodansparadis/vscp.git
```    

*Note* For a production system choosing a stable release version is strongly recommended.

You can also download a zip of the latest bleeding edge source by clicking on the 'clone and download' button at
[GitHub](https://github.com/grodansparadis/vscp).

Enter the 'vscp' folder created by the unpacking/clone process

####  Step 3

Enter the VSCP project folder ('vscp') and do

```bash
cd vscp
mkdir build
cd build
cmake ..
sudo make install
```

This builds amd installs the system.

The service should be installed and be started automatically. Id it does not use the following commands

```bash
sudo systemctl enable vscpd
sudo systemctl start vscpd
```

    
It is also possible to run the daemon as any other program with  

```bash
/usr/local/bin/vscpd -s
```

this is probably the best test to do on a newly installed system to see that the daemon starts properly as error messages will be shown in the terminal windows in this mode.  Terminate the daemon with **ctrl+c**  
    
Default install directory is */usr/sbin*

----

If you want to use SysVInit to SystemD instead of there is an init file in the service/sysvinit folder. Issue

```
make install_sysvinit
```

to install and then

```
sudo /etc/init.d/vscpd start to start
```

to start the service


## Setting up the system on Rasperry Pi

The [Raspberry Pi](https://www.raspberrypi.org/) is a very nice single board computer and a perfect match for VSCP & Friends. __The VSCP daemon should work on all Raspberry Pi's except the RPi1 with 256 MB memory where the SSL lib fails probably due to out of memory.__ 

If the startup script does not work or if you experience other startup problems you may need to update the firmware of your Raspberry Pi with **rpi-update**

VSCP & Friends has been tested and works very well on Raspberry Pi. 

The setup is the same as on other Unix systems so the instructions to [setup VSCP & Friends on a Linux system](./setting_up_the_system_on_linux.md) can be used. 



## Setting up the system on Beaglebone

[Beaglebone](https://beagleboard.org/bone) is a very nice board and VSCP & Friends is a perfect match for it if you are using distributions based on Debian wheezy or later.

VSCP & Friends has been tested and works very well on Beaglebone Black. The setup is the same as on other Unix systems so the instructions to [setup VSCP & Friends on a Unix system](./setting_up_the_system_on_linux.md) can be used.


### Related Links


*  opkg package manager - https://wiki.openwrt.org/doc/techref/opkg
*  Reflashing - http://circuitco.com/support/index.php?title=Updating_The_Software
*  Setting up CAN - https://github.com/digitalbond/canbus-beaglebone
*  Testing CAN - https://groups.google.com/forum/#!msg/beagleboard/MLJFSCh_J-w/fX1r6E_AvIoJ
* http://e2e.ti.com/support/arm/sitara_arm/f/791/t/154560.aspx?pi69795=2
*  Touch screen - http://www.carcomputer.co.uk/index.php?/monitors/hdmi-monitors/lilliput-fa1011-np/c/t-10.1-hdmi-touch-screen-monitor
*  


 
## Getting VSCPD working on an embedded ArchLinuxArm system

ArchLinux is a light-weight Linux distribution which provides a minimalistic framework to start with and gives the user full control on what he wants to do.
More information on ArchLinux: https://www.archlinux.org/

ArchLinuxArm is the port of ArchLinux packages to popular ARM devices and boards. The minimalistic nature of Arch makes it an excellent candidate to run on embedded devices.
More information on ArchLinuxArm: https://archlinuxarm.org/

To get up and running with an embedded board and Arch, it's advised to have an x86 host machine with Linux (any distribution will do) installed. This makes partitioning of the SD cards easier.

This guide has been tested with both a Beagleboard-Xm (TI) and a Wandboard (Freescale IMX). Both are ARMV7H architectures.

We're assuming that the embedded devices is running without a display and as such are only building **vscpd**.

### Step 1: Install ArchLinuxArm

The ArchLinuxArm website details the installation instructions for each supported target on their website. Examples for [Wandboard](https://archlinuxarm.org/platforms/armv7/freescale/wandboard) and [Beagleboard-Xm](https://archlinuxarm.org/platforms/armv7/ti/beagleboard-xm).

After installing, connect the board to your network (assuming DHCP). You can SSH to the board by using the default hostname "alarm" or alternatively, look up the IP address assigned by the DHCP server. The default user and password is also set to "alarm". Root password is "root".

If the connection fails, it's time to bring out a serial cable and see what's going wrong. Default serial port settings are 115200baud, 8bits, no parity. For Wandboard you'll need a null-modem serial cable.
    
### Step 2: Change hostname, user and password

Having default names for the host and users isn't the best way to have a secured system. So: [Change the hostname](https://wiki.archlinux.org/index.php/Network_configuration#Set_the_hostname),

[add a user](https://wiki.archlinux.org/index.php/users_and_groups#Example_adding_a_user) 

and [remove](https://wiki.archlinux.org/index.php/users_and_groups#Other_examples_of_user_management) the default 'alarm' user. Don't forget to change the root password!

Obviously, this is a good point to setup network settings, time & date (ntpd) etc etc. The [ArchLinux Wiki](https://wiki.archlinux.org/) is an excellent source of information and applicable to ArchLinuxArm since they're using the same packages.
    
### Step 3: Update and install required packages

    
One of the obvious differences to Debian based systems, is that Arch is using [pacman](https://wiki.archlinux.org/index.php/Pacman) (no, not the game) as a package manager. It takes some getting used to but is extremely user friendly. Rebuilding packages is a breeze as you'll see.

These commands need to be executed with root privileges, so:

```bash
    su
```    
    
and provide your root password.

To run a full system upgrade:

```bash
pacman -Syu
```

To install a package, simply enter:

```bash
pacman -S xxx
```

where xxx is the package name.

Following packages are required for this guide:

*  git
*  gcc
*  make
*  libpcap
*  fakeroot
*  screen  
    
Remember to exit root

```bash
exit
```
    
It's a good idea to reboot at this point.


###  Step 4: Get the VSCP sources

    
You can follow step 7 above or since we have GIT installed:

```bash
git clone https://github.com/grodansparadis/vscp.git
```

In a location you choose.

### Step 5: build VSCP

As root (su), do:

```bash
cd vscp_software
./configure --enable-ssl
```
    
Modify the makefile so that vscpd is not added to the init.d system, by commenting these two lines in the Makefile.

```bash 
#	$(INSTALL_PROGRAM) -b -m755 vscpd.startup_script_debian /etc/init.d/vscpd
#	update-rc.d vscpd defaults
```
    
Start the installation. Optionally, you can use **screen** again.

```bash
make install
```

### Step 6: install vscpd in systemd

Write the following, as root, to 

/etc/systemd/system/vscpd.service:

    [Unit]
    Description=Very Simple Control Protocol Daemon
    [Service] 
    Type=forking 
    PIDFile=/var/run/vscpd/vscpd.pid 
    ExecStart=-/usr/local/bin/vscpd
    TimeoutStopSec=1
    RuntimeDirectory=vscpd
    [Install]
    WantedBy=multi-user.target

Refresh systemd by running

```bash
systemctl daemon-reload
```
    
You can now start and stop vscpd by doing

```bash
systemctl start vscpd
systemctl stop vscpd
```
    
The service will be automatically started upon power up when you do

```bash
systemctl enable vscpd
```
    

## Installing vscpd on Windows

The current version 15.0 is not (yet) available on Windows. You can however use Windows Subsystem for Linux (WSL) to run it if you like. Install instructions is [here](http://localhost:3000). With the system installed follow the Linux install instructions.


## MQTT

From this version MQTT is central and the vscp server **must have** a MQTT broker to function. Initial configurations files use the Mosquitto test server (test.mosquitto.org:1883) which is open and free to use for testing. There are other [free and open brokers](https://mntolia.com/10-free-public-private-mqtt-brokers-for-testing-prototyping/) available for test. In a full production environment you should almost always setup your own broker or subscribe to some of the brokers that are available (such as the ones in the lin above).

To test, debug and do work with the vscp daemon server you also need a MQTT client. We use the [VSCP Works tool](https://github.com/grodansparadis/vscp-works-qt) for much of our work. This tool is currently being rewritten and MQTT support and other functionality is added as time goes.

For development we use the Mosquitto client tools. The main tools we use are [mosquitto_pub](https://mosquitto.org/man/mosquitto_pub-1.html) and [mosquitto_sub](https://mosquitto.org/man/mosquitto_sub-1.html). On a debian based system you can install them with

```bash
sudo apt install mosquitto-clients
```

Most samples in this documentation referee to these tools in some way.

Another MQTT tool that is useful to have installed is the [MQTT Explore](http://mqtt-explorer.com/).


[filename](./bottom_copyright.md ':include')
