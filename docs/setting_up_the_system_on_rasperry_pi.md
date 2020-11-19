![](./images/pi1l.jpg)

# Setting up the system on Rasperry Pi

The [Raspberry Pi](https://www.raspberrypi.org/) is a very nice single board computer and a perfect match for VSCP & Friends. __The VSCP daemon should work on all Raspberry Pi's except the RPi1 with 256 MB memory where the SSL lib fails probably due to out of memory.__ 

If the startup script does not work or if you experience other startup problems you may need to update the firmware of your Raspberry Pi with **rpi-update**

VSCP & Friends has been tested and works very well on Raspberry Pi. 

The setup is the same as on other Unix systems so the instructions to [setup VSCP & Friends on a Unix system](./setting_up_the_system_on_unix.md) can be used. 

As an alternative David Steeman have written easy to follow step by step instruction [here](http://www.steeman.be/posts/Installing%20VSCP%20on%20a%20Raspberry%20Pi). 

**IMPORTANT**
If you plan to use a Raspberry Pi in a control situation you will probably want to make the root file system read only. This is described [here](https://wiki.debian.org/ReadonlyRoot) and [here](https://learn.adafruit.com/read-only-raspberry-pi/).  If you don't do this you most certainly will find your board not starting after a power loss. Well you may be lucky for some time but eventually it will happen. So make the root partition read only!

Expect the build of VSCP to take half an hour on a Pi version 2 and an hour and a half on Pi version 1.

## Install vscpd

The easiest way to install vscpd is to install the armhf Debian package. The alternative option is to compile from source (see the [generic Unix installation](./setting_up_the_system_on_unix.md)).
    
Look up the armhf package in the [repository release section](https://github.com/grodansparadis/vscp/releases). When this is written the 14.0.0 release is the latest and the Debian package for Raspberry Pi is named **vscpd_14.0.0-1_armhf.deb** Installing this package with

```bash
    sudo dpkg -i vscpd_14.0.0-1_armhf.deb
    sudo apt-get install -f
```

or 

```bash
    sudo apt install vscpd_14.0.0-1_armhf.deb
```

the last method is [recommended](https://unix.stackexchange.com/questions/159094/how-to-install-a-deb-file-by-dpkg-i-or-by-apt).


After installing the package you can start the VSCP daemon with

```bash
systemctl start vscpd
```

Thats it! You can start to work with VSCP.


[filename](./bottom_copyright.md ':include')
 
