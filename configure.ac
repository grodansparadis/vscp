dnl Autoconf settings for VSCP
dnl $Id: configure.in,v 1.5 2017/10/13 11:22:06 akhe Exp $

AC_INIT( [vscp], [13.1.1], [akhe@grodansparadis.com])
AC_CONFIG_SRCDIR([src/vscp/common/canalsuperwrapper.cpp])
#AM_INIT_AUTOMAKE

CONFIGURE_LINE="$0 $*"
CODENAME="Ant"

AC_CONFIG_MACRO_DIR(m4)

AC_PREREQ(2.57)
AC_CONFIG_SRCDIR([src/vscp/common/canalsuperwrapper.cpp])
AC_CONFIG_AUX_DIR(autotools)
AC_CONFIG_HEADERS([config.h])

AX_CHECK_ENABLE_DEBUG()

# Checks for programs.
AC_PROG_CXX
AC_PROG_AWK
AC_PROG_YACC
AC_PROG_CC
AC_PROG_CPP
AC_PROG_LEX
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_RANLIB

# Check for needed libraries.
dnl AC_CHECK_LIB([curses], [initscr])
AC_CHECK_LIB([dl], [dlsym])
AC_CHECK_LIB([pthread], [pthread_create])
AC_CHECK_LIB([crypto], [MD5_Init])
AC_CHECK_LIB([m],[sin])
AC_CHECK_LIB([rt],[clock_gettime])
AC_CHECK_LIB([z], [gzopen])
dnl AC_CHECK_LIB([pcap], [pcap_open_live])
AC_CHECK_LIB([ssl], [SSL_free])
AC_CHECK_LIB([expat], [XML_ParserCreate])

# Checks for header files.
AC_HEADER_DIRENT
AC_HEADER_STDC
AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS([OS.h arpa/inet.h fcntl.h inttypes.h limits.h locale.h netdb.h netinet/in.h sgtty.h stddef.h stdint.h stdlib.h string.h strings.h sys/file.h sys/ioctl.h sys/socket.h sys/time.h sys/timeb.h syslog.h termios.h unistd.h openssl/md5.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_C_INLINE
AC_TYPE_MODE_T
AC_TYPE_OFF_T
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_HEADER_TIME
AC_STRUCT_TM
AC_C_VOLATILE


# Checks for library functions.
AC_FUNC_FORK
AC_PROG_GCC_TRADITIONAL
AC_FUNC_MALLOC
AC_FUNC_MEMCMP
AC_FUNC_REALLOC
AC_FUNC_SELECT_ARGTYPES
AC_FUNC_SETVBUF_REVERSED
AC_TYPE_SIGNAL
AC_FUNC_STAT
AC_FUNC_STRFTIME
AC_FUNC_STRTOD
AC_FUNC_VPRINTF
AC_CHECK_FUNCS([alarm bzero dup2 floor gethostbyname gethostname gethrtime gettimeofday inet_ntoa memchr memmove memset mkfifo pow select setlocale socket sqrt strcasecmp strchr strcspn strdup strerror strncasecmp strpbrk strrchr strspn strstr strtol strtoul])

dnl bigendian check
AC_C_BIGENDIAN(
     [AC_DEFINE([WORDS_BIGENDIAN], 1, [Define for endiness.])],
     [AC_DEFINE([WORDS_BIGENDIAN], 0, [Define for endiness.])])

dnl compile 32-bit/64-bit
AC_CHECK_SIZEOF([long])
AS_IF([test "$ac_cv_sizeof_long" -eq 8],
    [AC_DEFINE([OSBIT], 64, [Define to the size of a long in bits.])],
    [AC_DEFINE([OSBIT], 32, [Define to the size of a long in bits.])])

AC_SUBST(EXTRALIBS)
AC_SUBST(STATIC)
AC_SUBST(VSCPDIR)
AC_SUBST(RPI)

dnl
dnl Project base installation folder
dnl

AC_ARG_VAR(VSCPDIR, Set base folder for installation (/srv/vscp) )
if test "${VSCPDIR+set}" = set; then
  echo "variable \$VSCPDIR is set to: $VSCPDIR"
else
  VSCPDIR="/srv/vscp"
  echo "variable \$VSCPDIR is set to: $VSCPDIR"
fi

dnl
dnl Project Sub directories
dnl

PROJ_SUBDIRS="src/vscp/drivers/level1/can4vscp/linux\
 src/vscp/drivers/level1/can232/linux\
 src/vscp/drivers/level1/xap/linux\
 src/vscp/drivers/level1/socketcan/linux\
 src/vscp/drivers/level2/wire1/linux\
 src/vscp/drivers/level2/lmsensors/linux\
 src/vscp/drivers/level2/socketcan/linux\
 src/vscp/drivers/level2/mqtt/linux\
 src/vscp/drivers/level2/tcpdrv/linux\
 src/vscp/drivers/level2/rawethernet/linux\
 src/vscp/drivers/level2/sim/linux\
 src/vscp/linuxvscpl1\
 src/vscp/helperlib/linux\
 src/vscp/daemon/linux\
 src/vscp/vscp-mkpasswd\
 src/vscp/uvscpd\
 src/vscp/vscpcmd\
 src/vscp/relayd "

AC_ARG_ENABLE(rpi,
[  --enable-rpi         Build Raspberry Pi specific stuff (default disabled)],
[vscp_rpi=yes],[vscp_rpi=no])

test "${vscp_rpi}" == "yes"
if test "${vscp_rpi}" = yes ; then
    PROJ_SUBDIRS+=" src/vscp/drivers/level2/rpigpio/linux\
    src/vscp/drivers/level2/rpilcd/linux\
    src/vscp/drivers/level2/rpimax6675/linux"
    AC_CHECK_LIB([wiringPi], [wiringPiSetup])
fi

AC_SUBST(PROJ_SUBDIRS)

dnl
dnl  openssl version
dnl

if sh -c "openssl version | cut -d' ' -f 2 " 2>/dev/null | grep 1.1. >/dev/null; then
echo "openssl 1.1 libs will be used."
CFLAGS+=" -DOPENSSL_API_1_1"
else
echo "openssl 1.0 libs will be used."
CFLAGS+=" -DOPENSSL_API_1_0"
fi


dnl
dnl  Static build if wxWidgets
dnl

dnl WX AC_ARG_ENABLE(static,
dnl WX [  --enable-static         static build of wxWidgets (default shared)],
dnl WX [vscp_static=yes],[vscp_static=no])
dnl test "${vscp_static}" != "yes" && vscp_static="no"

dnl
dnl  wxWindows module
dnl


EXTRALIBS="${EXTRALIBS} ${LIBS}"

if test "${vscp_static}" = yes ; then
STATIC="yes"
else
STATIC="no"
fi

dnl
dnl Construct Makefiles
dnl

AC_CONFIG_FILES([Makefile
         m4/Makefile
         src/vscp/vscpcmd/Makefile
         src/vscp/daemon/linux/Makefile
         src/vscp/uvscpd/Makefile
         src/vscp/vscp-mkpasswd/Makefile
         src/vscp/drivers/level1/can232/linux/Makefile
         src/vscp/drivers/level1/xap/linux/Makefile
         src/vscp/drivers/level1/socketcan/linux/Makefile
         src/vscp/drivers/level1/can4vscp/linux/Makefile
         src/vscp/drivers/level2/lmsensors/linux/Makefile
	      src/vscp/drivers/level2/mqtt/linux/Makefile
         src/vscp/drivers/level2/socketcan/linux/Makefile
         src/vscp/drivers/level2/tcpdrv/linux/Makefile
         src/vscp/drivers/level2/rawethernet/linux/Makefile
         src/vscp/drivers/level2/wire1/linux/Makefile
	      src/vscp/drivers/level2/sim/linux/Makefile
         src/vscp/linuxvscpl1/Makefile
         src/vscp/helperlib/linux/Makefile
         src/vscp/relayd/Makefile
])

if test "${vscp_rpi}" = yes ; then
AC_CONFIG_FILES([src/vscp/drivers/level2/rpigpio/linux/Makefile
        src/vscp/drivers/level2/rpilcd/linux/Makefile
        src/vscp/drivers/level2/rpimax6675/linux/Makefile
])
fi

dnl Generate makefiles
AC_OUTPUT

dnl Output some debugging information
echo   "LIBS          : ${LIBS}"
dnl WX echo   "WXLIBS        : ${WXLIBS}"
echo   "EXTRALIBS     : ${EXTRALIBS}"
echo   "EXTRACPPFLAGS : ${CPPFLAGS}"
echo   "EXTRACXXFLAGS : ${CXXFLAGS}"
echo   "EXTRACFLAGS   : ${CFLAGS}"
echo   "PROJ_SUBDIRS  : ${PROJ_SUBDIRS}"

dnl Give user some feedback
echo
echo   "           VSCP configuration"
echo   "--------------------------------------------"
echo   "VSCP version          : `sh echo-version` (${PACKAGE_VERSION})"
echo   "System                : `uname -a` "
echo   "Architecture          : $(arch)"
echo   "VSCP GUI Build        : ${vscp_gui}"
echo   "VSCP install dir      : ${VSCPDIR}"

dnl echo   "${ax_enable_debug}"
if test "${ax_enable_debug}" = "yes"
then
   echo "build flavour         : debug "
else
   echo "build flavour         : release "
fi

if test "${vscp_static}" = "yes"
then
   echo "linking : static "
else
   echo "linking : shared "
fi

if test "${vscp_rpi}" = yes ; then
   echo   "Raspberry Pi build    : ${vscp_rpi}"
else
   echo   "Raspberry Pi build    : no"
fi

printf "
You can tune the compiler flags in vscp-config.
To build VSCP and friends, type \'make\' followed by \'make install\'

 use --enable-debug to build a debug version; release is default.
 use --enable-static to include a static version (currently diabled).
 use --enable-rpi to build Raspberry Pi specific drivers.

 \"make\" -- builds VSCP & Friends.
 \"make install\" -- builds VSCP & Friends and install system.
 \"make install-conf\" -- install/restore configuration files.
 \"make web-install\" -- install/restore web demo content including websocket examples.
 \"make deb\" -- builds VSCP & Friends Debian install files.
 \"make clean\" -- clean project.
"
dnl vi: set et sts=4 sw=4 com=\:dnl
