#!/bin/bash

# Install to use
# sudo apt-get install build-essential automake autoconf libtool pkg-config libcurl4-openssl-dev intltool libxml2-dev libgtk2.0-dev libnotify-dev libglib2.0-dev libevent-dev checkinstall

# get version from the version file
MAJOR_VERSION=`sed '35!d' ../common/version.h  | cut -b 33-`
MINOR_VERSION=`sed '36!d' ../common/version.h  | cut -b 33-`
RELEASE_VERSION=`sed '37!d' ../common/version.h  | cut -b 33-`
BUILD_VERSION=`sed '38!d' ../common/version.h  | cut -b 33-`
LIB_PLUS_VER="libvscphelper.so.$MAJOR_VERSION.$MINOR_VERSION.$RELEASE_VERSION"

#Build debian
checkinstall --type="debian" \
--pkgname="vscpworks" \
--install=no \
--pkgversion="$MAJOR_VERSION.$MINOR_VERSION.$RELEASE_VERSION" \
--pkgrelease="$BUILD_VERSION" \
--pkglicense="GPL2" \
--pkggroup="developer" \
--strip=yes \
--stripso=yes \
--addso=yes \
--gzman=yes \
--backup=no \
--pkgsource="https://github.com/grodansparadis/vscp" \
--maintainer="akhe@grodansparadis.com" \
--requires="libc6,libwxgtk3.0-0 \(\>=3.0~\) \| libwxgtk3.1-0 \(\>=3.1~\), libssl1.0.0 \| libssl1.0.1 \| libssl1.0.2"

#build rpm
#checkinstall --type="rpm" --pkgname="vscpd" --pkgversion="$MAJOR_VERSION.$MINOR_VERSION.$RELEASE_VERSION" --pkgrelease="1" --pkglicense="MIT" --pkggroup="developer" --pkgsource="https://github.com/grodansparadis/vscp" --maintainer="akhe@grodansparadis.com" --requires="libwxbase3.0-dev \(\>=3.0.0\) ,libssl-dev"
