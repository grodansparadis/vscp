#!/bin/sh

#   arg1 - platform
#   ---------------
#   i386    ununtu/debian    ./build_debian_package.sh i386 10
#   amd64   ununtu/debian    ./build_debian_package.sh amd64 10
#   armhf   raspbian         ./build_debian_package.sh armhf 10
#
#   arg2 - COMPAT

# Package version
MAJOR_VERSION=`head -n4  VERSION.m4 |  grep major_version | awk '{print $2}' | tr -d "[] ()"`
MINOR_VERSION=`head -n4  VERSION.m4 |  grep minor_version | awk '{print $2}' | tr -d "[] ()"`
RELEASE_VERSION=`head -n4  VERSION.m4 |  grep release_version | awk '{print $2}' | tr -d "[] ()"`
BUILD_VERSION=`head -n4  VERSION.m4 |  grep build_version | awk '{print $2}' | tr -d "[] ()"`
RELEASE_DEBIAN=`head -n4  VERSION.m4 |  grep release_debian | awk '{print $2}' | tr -d "[] ()"`

NAME_PLUS_VER="vscpd_$MAJOR_VERSION.$MINOR_VERSION.$RELEASE_VERSION"
#BUILD_FOLDER="dist/`date +vscp_build_%y%m%d_%H%M%S`"
rm -rf ../dist/*
DATENOW="`date -R`"
BUILD_FOLDER="../dist"

# Debian compability 10 on Raspberry
# relevant for 'control' and 'compat'
COMPAT="12"

case "$2" in
"")
    echo "Using discovered COMPAT"	
    ;;  
*)
    echo "Setting COMPAT = $2"
    COMPAT="$2"
    ;;
esac

# makes correct /usr/lib subfolder (/usr/lib/x86_64-linux-gnu/), none on Raspberry
# relevant for 'install' and 'links'
#SUBFOLDER='$(DEB_BUILD_GNU_CPU)-$(DEB_BUILD_GNU_SYSTEM)'
#SUBFOLDER="x86_64-linux-gnu"
SUBFOLDER=""

# dependencies for control
DEPENDENCY="libc6-dev (>= 2.14), libstdc++6 (>= 5.2), libgcc1 (>= 1:3.0), libssl-dev (>=1.0)"

# Get OS and version
if [ -f /etc/os-release ]; then
    # freedesktop.org and systemd
    . /etc/os-release
    OS=$NAME
    VER=$VERSION_ID
elif type lsb_release >/dev/null 2>&1; then
    # linuxbase.org
    OS=$(lsb_release -si)
    VER=$(lsb_release -sr)
elif [ -f /etc/lsb-release ]; then
    # For some versions of Debian/Ubuntu without lsb_release command
    . /etc/lsb-release
    OS=$DISTRIB_ID
    VER=$DISTRIB_RELEASE
elif [ -f /etc/debian_version ]; then
    # Older Debian/Ubuntu/etc.
    OS=Debian
    VER=$(cat /etc/debian_version)
elif [ -f /etc/SuSe-release ]; then
    # Older SuSE/etc.
    ...
elif [ -f /etc/redhat-release ]; then
    # Older Red Hat, CentOS, etc.
    ...
else
    # Fall back to uname, e.g. "Linux <version>", also works for BSD, etc.
    OS=$(uname -s)
    VER=$(uname -r)
fi

echo "OS=${OS} VER=${VER}"

# Get arcitecture
case $(uname -m) in
x86_64)
    BITS=64
    SUBFOLDER="x86_64-linux-gnu"
    ;;
i*86)
    BITS=32
    SUBFOLDER="i386-linux-gnu"
    ;;
armv6l)
    BITS=32
    SUBFOLDER=""
    ;; 
armv7l)
    BITS=64
    SUBFOLDER=""
    ;;       
*)
    BITS=?
    ;;
esac

# Get word width
echo "BITS=${BITS}"

case $(uname -m) in
x86_64)
    ARCH=x64  # or AMD64 or Intel64 or whatever
    ;;
i*86)
    ARCH=x86  # or IA32 or Intel32 or whatever
    ;;
*)
    # leave   armv6l/armv7l
    ARCH=`uname -m`
    ;;
esac

echo "ARCH=${ARCH}"

case $OS in
Debian)
    echo "11"
    echo "22"
    ;;
Ubuntu)
    echo "11"
    echo "22"
    ;;
Raspian)
    echo "111"
    echo "222"
    ;;
esac

# ---------------------------------------------------------------------

case "$2" in
"")
    echo "Using discovered COMPAT"	
    ;;  
*)
    echo "Setting COMPAT = $2"
    COMPAT="$2"
    ;;
esac

# ---------------------------------------------------------------------

case "$1" in
amd64)
    echo "Building for amd64"	
    BITS=64
    SUBFOLDER="x86_64-linux-gnu"
    ARCH=x64
    ;;
i386)
    echo "Building for i386"	
    BITS=32
    SUBFOLDER="i386-linux-gnu"
    ARCH=x86
    ;;
armhf)
    echo "Building for armhf"	
    BITS=32
    SUBFOLDER="arm-linux-gnueabihf"
    ARCH=armhf
    ;;
*)
    echo "Building with discovery for installed system"
    ;;
esac


# ---------------------------------------------------------------------


echo "***   ---$NAME_PLUS_VER"

# Create the build folder
echo "***   ---Creating build folder:"$BUILD_FOLDER
mkdir -p $BUILD_FOLDER

# Clean project
make clean
vscp/clean_for_dist
./clean_for_dist

echo "---Copying Debian_orig to destination folder"
cp -r debian_orig/ $BUILD_FOLDER

echo "***   ---making tar"
tar -zcf $BUILD_FOLDER/$NAME_PLUS_VER.orig.tar.gz *

echo "***   $NAME_PLUS_VER.tgz created."
cd $BUILD_FOLDER
mkdir $NAME_PLUS_VER/
cd $NAME_PLUS_VER/
tar -zxf ../$NAME_PLUS_VER.orig.tar.gz

echo "***   Making 'debian' folder"
mkdir debian
cp -r debian_orig/* debian

echo "***   Running dh_make"
dh_make --single --defaultless -f ../$NAME_PLUS_VER.tar.gz -a -s -c mit -y

echo "***   Do variable substitution"
sed -i "s/%MAJOR-VERSION/${MAJOR_VERSION}/g" debian/* 
sed -i "s/%MINOR-VERSION/${MINOR_VERSION}/g" debian/*
sed -i "s/%RELEASE-VERSION/${RELEASE_VERSION}/g" debian/*
sed -i "s/%BUILD-VERSION/${BUILD_VERSION}/g" debian/*
sed -i "s/%RELEASE-DEBIAN/${RELEASE_DEBIAN}/g" debian/*
sed -i "s/%COMPAT/${COMPAT}/g" debian/*
sed -i "s/%SUBFOLDER/${SUBFOLDER}/g" debian/*
sed -i "s/%DATENOW/${DATENOW}/g" debian/*

echo "***   ---Now do 'dpkg-buildpackage -us -uc' or 'dpkg-buildpackage -b'"

#cd $NAME_PLUS_VER
#debuild clean
debuild -us -uc

echo "***   If all is alright check dist folder for Debian package "


