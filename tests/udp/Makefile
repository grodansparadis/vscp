
CC = `wx-config --cc`
CXX = `wx-config --cxx`
MONGOOSE_CFLAGS = -DMG_ENABLE_THREADS -DMG_ENABLE_GETADDRINFO -UMG_ENABLE_SSL
CFLAGS = -std=c99 -g -O0 -D MG_ENABLE_THREADS -I../../src/vscp/common -I../../src/common -D_BSD_SOURCE
#CFLAGS = -g -O0 -pthread -D MG_ENABLE_THREADS -I../../src/vscp/common -I../../src/common
CXXFLAGS = -g -O0 
CPPFLAGS =  -I../../src/vscp/common -I../../src/common
CPPFLAGS += `wx-config --cxxflags base `
CPPFLAGS += $(MONGOOSE_CFLAGS)
LDFLAGS = 
EXTRALIBS =  -lz -lrt -lm -lcrypto -lpthread
EXTRALIBS += `wx-config --libs base`
OBJ = crc.o crc8.o guid.o aes.o vscphelper.o vscpremotetcpif.o mongoose.o  

all:	uc udpsrv send_vscp_udp_frame receive_vscp_udp_frame

uc:	udpclient.c
	cc -std=c99 $(CFLAGS) udpclient.c -o uc 

udpsrv:     udpsrv.c
	cc -std=c99 udpsrv.c -o udpsrv

vscphelper.o: ../..//src/vscp/common/vscphelper.o ../../src/vscp/common/vscphelper.h
	$(CXX)  $(CXXFLAGS) $(CPPFLAGS) -c ../../src/vscp/common/vscphelper.cpp -o $@

vscpremotetcpif.o: ../../src/vscp/common/vscpremotetcpif.o ../../src/vscp/common/vscpremotetcpif.h
	$(CXX)  $(CXXFLAGS) $(CPPFLAGS) -c ../../src/vscp/common/vscpremotetcpif.cpp -o $@

guid.o: ../../src/vscp/common/guid.o ../../src/vscp/common/guid.h
	$(CXX)  $(CXXFLAGS) $(CPPFLAGS) -c ../../src/vscp/common/guid.cpp -o $@	

crc.o: ../../src/common/crc.o ../../src/common/crc.h 
	$(CC) $(CFLAGS) -c ../../src/common/crc.c -o $@	

crc8.o: ../../src/common/crc8.o ../../src/common/crc8.h
	$(CC) $(CFLAGS) -c ../../src/common/crc8.c -o $@

aes.o: ../../src/common/aes.o ../../src/common/aes.h 
	$(CC) $(CFLAGS) -c ../../src/common/aes.c -o $@

mongoose.o: ../../src/common/mongoose.o ../../src/common/mongoose.h
	$(CC) $(CFLAGS) $(MONGOOSE_CFLAGS) -c ../../src/common/mongoose.c -o $@			

send_vscp_udp_frame:     send_vscp_udp_frame.o $(OBJ)
	$(CXX)  $(CFLAGS) -o $@ send_vscp_udp_frame.o $(OBJ) $(LDFLAGS) -L$(LIBDIRNAME) $(EXTRALIBS)	

receive_vscp_udp_frame:     receive_vscp_udp_frame.o $(OBJ)
	$(CXX)  $(CFLAGS) -o $@ receive_vscp_udp_frame.o $(OBJ) $(LDFLAGS) -L$(LIBDIRNAME) $(EXTRALIBS)

clean:
	rm -f uc
	rm -f udpsrv
	rm -f send_vscp_udp_frame
	rm -f receive_vscp_udp_frame
	rm -f *.o
