
CC = `wx-config --cc`
CXX = `wx-config --cxx`
CFLAGS = -std=c99 -g -O0 -D_BSD_SOURCE  -I../../src/vscp/common -I../../src/common
CXXFLAGS = -g -O0 
CPPFLAGS =  -I../../src/vscp/common -I../../src/common 
LDFLAGS = 
EXTRALIBS =  -lz  
OBJ = crc.o crc8.o aes.o 

all:	uc udpsrv send_vscp_udp_frame receive_vscp_udp_frame

uc:	udpclient.c
	cc -std=c99 $(CFLAGS) udpclient.c -o uc 

udpsrv:     udpsrv.c
	cc -std=c99 udpsrv.c -o udpsrv

crc.o: ../../src/common/crc.o ../../src/common/crc.h 
	$(CC) $(CFLAGS) -c ../../src/common/crc.c -o $@	

crc8.o: ../../src/common/crc8.o ../../src/common/crc8.h
	$(CC) $(CFLAGS) -c ../../src/common/crc8.c -o $@

aes.o: ../../src/common/aes.o ../../src/common/aes.h 
	$(CC) $(CFLAGS) -c ../../src/common/aes.c -o $@			

send_vscp_udp_frame:     send_vscp_udp_frame.o $(OBJ)
	$(CXX)  $(CPPFLAGS) -o $@  send_vscp_udp_frame.o $(OBJ) $(LDFLAGS) -L$(LIBDIRNAME) -L/usr/lib64 -lstdc++ $(EXTRALIBS)	

receive_vscp_udp_frame:     receive_vscp_udp_frame.o $(OBJ)
	$(CXX)  $(CPPFLAGS) -o $@ receive_vscp_udp_frame.o  $(OBJ) $(LDFLAGS) -L$(LIBDIRNAME) $(EXTRALIBS)

clean:
	rm -f uc
	rm -f udpsrv
	rm -f send_vscp_udp_frame
	rm -f receive_vscp_udp_frame
	rm -f *.o
