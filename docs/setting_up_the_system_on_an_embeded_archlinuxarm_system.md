## Getting VSCPD working on an embedded ArchLinuxArm system

ArchLinux is a light-weight Linux distribution which provides a minimalistic framework to start with and gives the user full control on what he wants to do.
More information on ArchLinux: https://www.archlinux.org/

ArchLinuxArm is the port of ArchLinux packages to popular ARM devices and boards. The minimalistic nature of Arch makes it an excellent candidate to run on embedded devices.
More information on ArchLinuxArm: https://archlinuxarm.org/

To get up and running with an embedded board and Arch, it's advised to have an x86 host machine with Linux (any distribution will do) installed. This makes partitioning of the SD cards easier.

This guide has been tested with both a Beagleboard-Xm (TI) and a Wandboard (Freescale IMX). Both are ARMV7H architectures.

We're assuming that the embedded devices is running without a display and as such are only building **vscpd**.

## Step 1: Install ArchLinuxArm

The ArchLinuxArm website details the installation instructions for each supported target on their website. Examples for [Wandboard](https://archlinuxarm.org/platforms/armv7/freescale/wandboard) and [Beagleboard-Xm](https://archlinuxarm.org/platforms/armv7/ti/beagleboard-xm).

After installing, connect the board to your network (assuming DHCP). You can SSH to the board by using the default hostname "alarm" or alternatively, look up the IP address assigned by the DHCP server. The default user and password is also set to "alarm". Root password is "root".

If the connection fails, it's time to bring out a serial cable and see what's going wrong. Default serial port settings are 115200baud, 8bits, no parity. For Wandboard you'll need a null-modem serial cable.
    
## Step 2: Change hostname, user and password

Having default names for the host and users isn't the best way to have a secured system. So: [Change the hostname](https://wiki.archlinux.org/index.php/Network_configuration#Set_the_hostname),

[add a user](https://wiki.archlinux.org/index.php/users_and_groups#Example_adding_a_user) 

and [remove](https://wiki.archlinux.org/index.php/users_and_groups#Other_examples_of_user_management) the default 'alarm' user. Don't forget to change the root password!

Obviously, this is a good point to setup network settings, time & date (ntpd) etc etc. The [ArchLinux Wiki](https://wiki.archlinux.org/) is an excellent source of information and applicable to ArchLinuxArm since they're using the same packages.
    
## Step 3: Update and install required packages

    
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

##  Step 4: Rebuild & upgrade wxbase

At the time of writing, the provided wxbase library contains one or more errors.
The workaround is to simply rebuild wxbase on your target.
If you want to proceed with the provided wxbase package, you can simply run **pacman -S wxbase** and proceed to step 6. Let us know how it worked out!

To rebuild wxbase:

```bash
git clone git://github.com/archlinuxarm/PKGBUILDs.git
cd PKGBUILDs/aur/wxbase
```
    
edit the PKGBUILD file 

```bash
nano PKGBUILD
```
    
and add *arch=('armv7h')* to the file, after the other parameters.

Leave nano (CTRL+X) and save the edited file.

Start the build process, using screen:

```bash
screen makepkg
```

This build will take a while. The advantage of running this in [screen](https://wiki.archlinux.org/index.php/GNU_Screen) is that you can close the session while the build continues. To exit the screen session, do **CTRL+A** followed by **CTRL+D**. You can now log off or do other stuff. To reconnect, do

```bash
screen -ls
```

to get a list of active sessions and

```bash
screen -r *session*
```

to reconnect to a session. If there's only one active, you can omit the session number. If the build completed, the session is closed.

In order to speed up the process, you can try to setup a DISTCC environment as described on the ArchLinuxArm website, but this is not trivial.

To install the freshly built library, do

```bash
pacman -U wxbase-3.0.2-3-armv7h.pkg.tar.xz
```

##  Step 5: Get the VSCP sources

    
You can follow step 7 above or since we have GIT installed:

```bash
git clone https://github.com/grodansparadis/vscp_software.git
```

In a location you choose.

## Step 6: build VSCP

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

## Step 7: install vscpd in systemd

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
    
The service will be automatically started upon powerup when you do

```bash
systemctl enable vscpd
```
    

[filename](./bottom_copyright.md ':include')