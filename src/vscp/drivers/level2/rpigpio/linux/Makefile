# =========================================================================
#                              Makefile.in         
# =========================================================================

# STATIC = yes if a static build is requested.
STATIC=no
VSCP_PROJ_BASE_DIR=/srv/vscp

prefix = /usr/local
exec_prefix = ${prefix}
INSTALL = /usr/bin/install -c
top_srcdir = ../../../../../..
top_wxsrcdir = @top_wxsrcdir@
top_builddir = ../../../../../..
top_wxbuilddir = @top_wxbuilddir@


CC = gcc
CXX = g++

# wxBase flags and libs
WXBASE_CPPFLAGS = -I/usr/lib/x86_64-linux-gnu/wx/include/gtk2-unicode-3.0 -I/usr/include/wx-3.0 -D_FILE_OFFSET_BITS=64 -DWXUSINGDLL -D__WXGTK__ -DwxUSE_GUI=0
WXBASE_CXXFLAGS = -I/usr/lib/x86_64-linux-gnu/wx/include/gtk2-unicode-3.0 -I/usr/include/wx-3.0 -D_FILE_OFFSET_BITS=64 -DWXUSINGDLL -D__WXGTK__ -DwxUSE_GUI=0 -pthread
WXBASE_CFLAGS = -I/usr/lib/x86_64-linux-gnu/wx/include/gtk2-unicode-3.0 -I/usr/include/wx-3.0 -D_FILE_OFFSET_BITS=64 -DWXUSINGDLL -D__WXGTK__ -DwxUSE_GUI=0 -pthread
WXBASE_LIBS = -L/usr/lib/x86_64-linux-gnu -pthread   -lwx_baseu-3.0 -lwx_baseu_xml-3.0 -lwx_baseu_net-3.0 
WXBASE_STATIC_LIBS = 

# wxGTK flags and libs
WXGTK_CPPFLAGS = -I/usr/lib/x86_64-linux-gnu/wx/include/gtk2-unicode-3.0 -I/usr/include/wx-3.0 -D_FILE_OFFSET_BITS=64 -DWXUSINGDLL -D__WXGTK__
WXGTK_CXXFLAGS = -I/usr/lib/x86_64-linux-gnu/wx/include/gtk2-unicode-3.0 -I/usr/include/wx-3.0 -D_FILE_OFFSET_BITS=64 -DWXUSINGDLL -D__WXGTK__ -pthread
WXGTK_CFLAGS = -I/usr/lib/x86_64-linux-gnu/wx/include/gtk2-unicode-3.0 -I/usr/include/wx-3.0 -D_FILE_OFFSET_BITS=64 -DWXUSINGDLL -D__WXGTK__ -pthread
WXGTK_LIBS = -L/usr/lib/x86_64-linux-gnu -pthread   -lwx_gtk2u_xrc-3.0 -lwx_gtk2u_html-3.0 -lwx_gtk2u_qa-3.0 -lwx_gtk2u_adv-3.0 -lwx_gtk2u_core-3.0 -lwx_baseu_xml-3.0 -lwx_baseu_net-3.0 -lwx_baseu-3.0 
WXGTK_STATIC_LIBS = 

# get version from the version file
MAJOR_VERSION = `sed '35!d' ../../../../common/version.h  | cut -b 33-`
MINOR_VERSION = `sed '36!d' ../../../../common/version.h  | cut -b 33-`
RELEASE_VERSION = `sed '37!d' ../../../../common/version.h  | cut -b 33-`
BUILD_VERSION = `sed '38!d' ../../../../common/version.h  | cut -b 33-

CFLAGS = -std=c99  -g -O0 -DOPENSSL_API_1_1 -fPIC -I/usr/lib/x86_64-linux-gnu/wx/include/gtk2-unicode-3.0 -I/usr/include/wx-3.0 -D_FILE_OFFSET_BITS=64 -DWXUSINGDLL -D__WXGTK__ -DwxUSE_GUI=0 -pthread -DCBC -I../../../../common -I../../../../../common
CXXFLAGS = -std=c++11 -D__LINUX__ -fPIC -D_REENTRANT  -g -O0 -I/usr/lib/x86_64-linux-gnu/wx/include/gtk2-unicode-3.0 -I/usr/include/wx-3.0 -D_FILE_OFFSET_BITS=64 -DWXUSINGDLL -D__WXGTK__ -DwxUSE_GUI=0 -pthread
CPPFLAGS = -D__LINUX__ -fPIC  -I/usr/lib/x86_64-linux-gnu/wx/include/gtk2-unicode-3.0 -I/usr/include/wx-3.0 -D_FILE_OFFSET_BITS=64 -DWXUSINGDLL -D__WXGTK__ -DwxUSE_GUI=0 -fno-var-tracking-assignments -I. -I../common -I../../../../common -I../../../../../common

LDFLAGS = 
EXTRALIBS = -lpigpio -lpigpiod_if2 -lexpat  -lssl -lz -lrt -lm -lcrypto -lpthread -ldl  
ifeq ($(STATIC),yes)
EXTRALIBS += 
else
EXTRALIBS += -L/usr/lib/x86_64-linux-gnu -pthread   -lwx_baseu-3.0 -lwx_baseu_xml-3.0 -lwx_baseu_net-3.0 
endif

DLFLAGS = -g -shared -fPIC
STRIP = strip
INSTALL_PROGRAM = ${INSTALL}
INSTALL_DATA = ${INSTALL} -m 644
INSTALL_DIR = /usr/bin/install -c -d
srcdir = .
top_srcdir = .
top_builddir =
bindir = ${exec_prefix}/bin
libdir = ${exec_prefix}/lib
datadir = ${prefix}/share
includedir = ${prefix}/include
DLLPREFIX = lib


### Variables: ###

WX_RELEASE = 3.0
LIBDIRNAME = $(top_wxbuilddir)lib

VSCPRPIGPIO_OBJECTS = vscpl2drv-rpigpio.o\
	vscpremotetcpif.o\
	vscphelper.o\
	sockettcp.o\
	guid.o\
	crc.o\
	crc8.o\
	vscp_aes.o\
	vscpmd5.o\
	fastpbkdf2.o\
	vscpbase64.o

### Targets: ###

all: rpigpio.o vscpl2drv-rpigpio.so vscpl2drv-rpigpio-if2.so test-if2.o test test-if2

test:  test.o rpigpio.o $(VSCPRPIGPIO_OBJECTS)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -o $@ test.o rpigpio.o $(VSCPRPIGPIO_OBJECTS) $(LDFLAGS) -L$(LIBDIRNAME) $(WXLIBS) $(EXTRALIBS)

test-if2:  test-if2.o rpigpio-if2.o $(VSCPRPIGPIO_OBJECTS)
	$(CXX) -DUSE_PIGPIOD $(CXXFLAGS) $(CPPFLAGS) -o test-if2 test-if2.o rpigpio-if2.o $(VSCPRPIGPIO_OBJECTS) $(LDFLAGS) -L$(LIBDIRNAME) $(WXLIBS) $(EXTRALIBS)

test-if2.o:  test.o 
	$(CXX) -c test.cpp -DUSE_PIGPIOD $(CXXFLAGS) $(CPPFLAGS) -o test-if2.o 

# Build the Linux DL
vscpl2drv-rpigpio.so: $(VSCPRPIGPIO_OBJECTS)
	$(CXX) -o $@  $(VSCPRPIGPIO_OBJECTS) rpigpio.o $(DLFLAGS) -L$(LIBDIRNAME) $(WXLIBS) $(EXTRALIBS) 

vscpl2drv-rpigpio-if2.so: $(VSCPRPIGPIO_OBJECTS)
	$(CXX) -o $@ $(VSCPRPIGPIO_OBJECTS) rpigpio-if2.o $(DLFLAGS) -L$(LIBDIRNAME) $(WXLIBS) $(EXTRALIBS)	

vscpl2drv-rpigpio.o: vscpl2drv-rpigpio.cpp vscpl2drv-rpigpio.h
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c vscpl2drv-rpigpio.cpp -o $@

rpigpio.o: rpigpio.cpp rpigpio.h
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c rpigpio.cpp -o $@
	$(CXX) -DUSE_PIGPIOD $(CXXFLAGS) $(CPPFLAGS) -c rpigpio.cpp -o rpigpio-if2.o

vscphelperlib.o: ../../../../vscphelperlib.cpp ../../../../vscphelperlib.h
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c ../../../../vscphelperlib.cpp -o $@

vscpremotetcpif.o: ../../../../common/vscpremotetcpif.cpp ../../../../common/vscpremotetcpif.h
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c ../../../../common/vscpremotetcpif.cpp -o $@

vscphelper.o: ../../../../common/vscphelper.cpp ../../../../common/vscphelper.h
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c ../../../../common/vscphelper.cpp -o $@

sockettcp.o: ../../../../../common/sockettcp.c ../../../../../common/sockettcp.h
	$(CC) $(CFLAGS) -c ../../../../../common/sockettcp.c -o $@

guid.o: ../../../../common/guid.cpp ../../../../common/guid.h
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c ../../../../common/guid.cpp -o $@	
	
vscp_aes.o: ../../../../../common/vscp_aes.c ../../../../../common/vscp_aes.h
	$(CC) $(CFLAGS) -c ../../../../../common/vscp_aes.c -o $@

crc.o: ../../../../../common/crc.c ../../../../../common/crc.h
	$(CC) $(CFLAGS) -c ../../../../../common/crc.c -o $@
	
crc8.o: ../../../../../common/crc8.c ../../../../../common/crc8.h
	$(CC) $(CFLAGS) -c ../../../../../common/crc8.c -o $@		

vscpbase64.o: ../../../../../common/vscpbase64.c ../../../../../common/vscpbase64.h
	$(CC) $(CFLAGS) -c ../../../../../common/vscpbase64.c -o $@

fastpbkdf2.o: ../../../../../common/fastpbkdf2.c ../../../../../common/fastpbkdf2.h
	$(CC) $(CFLAGS) -c ../../../../../common/fastpbkdf2.c -o $@

vscpmd5.o: ../../../../../common/vscpmd5.c ../../../../../common/vscpmd5.h
	$(CC) $(CFLAGS) -c ../../../../../common/vscpmd5.c -o $@	

dllist.o: ../../../../../common/dllist.c ../../../../../common/dllist.h
	$(CC) $(CFLAGS) -c ../../../../../common/dllist.c -o $@	

install: all
	mkdir -p $(VSCP_PROJ_BASE_DIR)/drivers/level2
	$(INSTALL_PROGRAM) vscpl2drv-rpigpio.so $(VSCP_PROJ_BASE_DIR)/drivers/level2
	$(INSTALL_PROGRAM) vscpl2drv-rpigpio-if2.so $(VSCP_PROJ_BASE_DIR)/drivers/level2

uninstall: 

install-strip: install

deb:    all
	echo "\n" | ./builddist.sh
	@mkdir -p ../../../../../../dist
	@mv *.deb ../../../../../../dist

clean: 
	rm -rf ./.deps ./.pch
	rm -f ./*.o
	rm -f ../common/*.o
	rm -f ../../../common/*.o
	rm -f ../../../../common/*.o
	rm -f ../../../../../common/*.o
	rm -f vscpl2drv-rpigpio.so
	rm -f vscpl2drv-rpigpio-if2.so
	rm -f test
	rm -f test-if2
	rm -f *.deb
	rm -f *.gz

distclean: clean
	rm -f config.cache config.log config.status bk-deps bk-make-pch shared-ld-sh Makefile

$(ALWAYS_BUILD):  .FORCE

.FORCE: 

# Include dependency info, if present:
-include .deps/*.d

.PHONY: all install uninstall clean distclean data .FORCE



