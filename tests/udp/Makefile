
CC = cc
CXX = gcc

CFLAGS = -std=c99 -g -O0 -I../../src/vscp/common -I../../src/common -D_BSD_SOURCE
CXXFLAGS = -g -O0 
CPPFLAGS =  -I../../src/vscp/common -I../../src/common 
LDFLAGS = 
EXTRALIBS =  -lz -lrt -lm -lcrypto

TEST_SPECIALS = crc.o

all:	uc udpsrv send_vscp_udp_frame receive_vscp_udp_frame

uc:	udpclient.c
	cc -std=c99 $(CFLAGS) udpclient.c -o uc 

udpsrv:     udpsrv.c
	cc -std=c99 udpsrv.c -o udpsrv

crc.o: ../../src/common/crc.o ../../src/common/crc.h 
	$(CC) $(CFLAGS) -c ../../src/common/crc.c -o $@	

send_vscp_udp_frame:     send_vscp_udp_frame.o crc.o
	$(CC)  $(CFLAGS) -o $@ send_vscp_udp_frame.o crc.o $(LDFLAGS) -L$(LIBDIRNAME) $(EXTRALIBS)	

receive_vscp_udp_frame:     receive_vscp_udp_frame.o crc.o
	$(CC)  $(CFLAGS) -o $@ receive_vscp_udp_frame.o crc.o $(LDFLAGS) -L$(LIBDIRNAME) $(EXTRALIBS)

clean:
	rm -f uc
	rm -f udpsrv
	rm -f send_vscp_udp_frame
	rm -f receive_vscp_udp_frame
	rm -f *.o
