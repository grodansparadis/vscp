# Setting up the system on Unix

To install VSCP & Friends on Unix you need to install the binary package or build the system from source. The build process is simple and it usually don't give any problems. You can find binary and source files [here](https://github.com/grodansparadis/vscp/releases)

## Installing from binary package

The binary package is supplied as a Debian package. There are several variants available. 

**x86_64** should work on a Debian/Ubuntu or other Debian derived 64-bit system.

**amd64** should work on a Debian/Ubuntu or other Debian derived 32-bit system.

**arm7hf** should work on any arm7 system like Raspberry Pi.

### Debian package installation
This should work as an installation method on all distribution's that can handle Debian packages.

You find the Debian package [here](https://github.com/grodansparadis/vscp/releases). Download the package for your architecture and install with

When this is written the 14.0.0 release is the latest and the Debian package for Raspberry Pi is named **vscpd_14.0.0-1_amd64.deb** Installing this package with

```bash
    sudo dpkg -i vscpd_14.0.0-1_amd64.deb
    sudo apt-get install -f
```

or 

```bash
    sudo apt install vscpd_14.0.0-1_amd64.deb
```

the last method is [recommended](https://unix.stackexchange.com/questions/159094/how-to-install-a-deb-file-by-dpkg-i-or-by-apt).


After installing the package you can start the VSCP daemon with

```bash
systemctl start vscpd
```

Thats it! You can start to work with VSCP.

### Private Debian package repository - Experimental

**Note:** Use only for testing !!!!

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


## Building binaries from source

### Step 1

Prepare the build environment with

```bash
sudo apt update
sudo apt install build-essential
sudo apt install git libssl-dev libexpat-dev libsystemd-dev libwrap0-dev libz-dev libmosquitto-dev
```    
to install the build tools. 

### Step 2

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

You can also download a zip of the latest bleeding edge source by clicking ion the 'clone and download' button at
[GitHub](https://github.com/grodansparadis/vscp).

Enter the 'vscp' folder created by the unpacking/clone process

###  Step 3

Enter the VSCP project folder ('vscp') and do

```bash
cd vscp
./configure
bash
```


If you get warnings about 

    ERR_remove_state(0);
    
being deprecated   

use 

```bash
./configure CFLAGS="-DOPENSSL_API_1_1"
```

to force use of openssl 1.1 instead of openssl 1.0

You can find out which version of openssl you have installed with

```bash
openssl version
```

```bash    
ls /usr/lib/i386-linux-gnu/libssl.so* (pc)
ls /usr/lib/arm-linux-gnueabihf/libssl.so* (RPi)
```    

{{after_configure.png?600}}   

#### Debugging

If you want a debug build use the flag

```bash	
--enable-debug
```

This flags used without a value is the same as

	
	--enable-debug="yes"


you can specify values as below

 | Value   | Description | 
 | -----   | ----------- | 
 | yes     | Specifying 'yes' adds '-g -O0' to the compilation flags for all languages. | 
 | info    | Specifying 'info' adds '-g' to the compilation flags. | 
 | profile | Specifying 'profile'adds '-g -pg' to the compilation flags and '-pg' to the linking flags. | 

### Step 4

```bash
make
```    

To make sure everything build OK you can issue make again and check the output for errors. You don't get bombarded with such masses of text on the second run. 

__You may get some complaints when the code is compiled. This is normally nothing to worry about and they are just warnings from subcomponents.__

Tip    
It can be hard to see errors in all the text that is output by the build process. If you use

```bash
make | grep error
```    

it is much easier so see possible errors during the build process.  

    
###  Step 5

Install the system with

```bash
make install
```

Instead of first doing *make* and then *make install* you can of course do *make install* directly.

```bash
make install-conf
```

installs the default configuration files. Can always be used to restore them also.  

```bash
sudo make install-systemd-script
```

installs systemd startup scripts.

Reboot or start service with

```
sudo systemctl start vscpd
```

```bash
make web-install
```

installs default web content.  

This is done with script 

```bash
./do_web_download
```

which downloads and installs the latest UX code in the default folder. This script is located in the project root folder and can be used without the makefile also.

###  Step 6

You can now remove the source packages if your want.
    
After doing this you can set up a working VSCP & Friends system.

###  Step 7

 
Now reboot and the daemon will run. or issue

```bash
sudo systemctl enable vscpd
sudo systemctl start vscpd
```

to start the server.  
    
It is also possible to run the daemon as any other program with  

```bash
/usr/local/bin/vscpd -s
```

this is probably the best test to do on a newly installed system to see that the daemon starts properly as error messages will be shown in the terminal windows in this mode.  Terminate the daemon with **ctrl+c**  
    
Default install directory is */usr/local/sbin*

----

### Notes

If your system does not have C++11 (Ubuntu Trusty) see this https://askubuntu.com/questions/428198/getting-installing-gcc-g-4-9-on-ubuntu  

----

If you want to use SysVInit to SystemD instead of there is an init file in the service/sysvinit folder. Issue

```
make install_sysvinit
```

to install and then

```
sudo /etc/init.d/vscpd start to start
```

to start te service

----

vscpd uses [Google test](https://github.com/google/googletest) for testing. If you want to build support for it do the following when in the VSCP root folder

```bash
git submodule update --recursive --remote
mkdir build
cd build
cmake ..
```



[filename](./bottom_copyright.md ':include')
