
# Debug or Release
CONFIG = -g3 -ggdb -O0 -DDEBUG=1
# # CONFIG = -g2 -Os -DNDEBUG=1

OUR_CFLAGS = $(CONFIG) -std=c99 -Wall -Wextra -Wconversion -Wformat -Wformat=2 -Wformat-security -Wno-deprecated-declarations -Wno-unused-function



CFLAGS = -std=c99 -g -O0

all:	example1 send1 send_periodic bigtest stcptest simple_tls_server simple_tls_client sockettcp.o

sockettcp.o: ../../../../common/sockettcp.c ../../../../common/sockettcp.h
	$(CC) $(CFLAGS) -c ../../../../common/sockettcp.c -o $@ -I../../../common -I../../../../common

stcptest:      stcptest.c sockettcp.o
	        $(CC)  $(CFLAGS)  stcptest.c -o stcptest  -I../../../common -I../../../../common -Wl,-rpath,/home/akhe/development/VSCP/vscp/src/vscp/helperlib/linux -L/home/akhe/development/VSCP/vscp/src//vscp/helperlib/linux -lssl -lcrypto  -lm  -lvscphelper

example1:	example1.c
	$(CC) $(CFLAGS) example1.c -o example1 -I ../.. -I../../../common -I../../../../common -Wl,-rpath,/home/akhe/development/VSCP/vscp/src/vscp/helperlib/linux -L/home/akhe/development/VSCP/vscp/src//vscp/helperlib/linux -lvscphelper

send1:       send1.c
	        $(CC) $(CFLAGS) send1.c -o send1 -I ../.. -I../../../common -I../../../../common -Wl,-rpath,/home/akhe/development/VSCP/vscp/src/vscp/helperlib/linux -L/home/akhe/development/VSCP/vscp/src//vscp/helperlib/linux -lvscphelper

send_periodic:       send_periodic.c
	        $(CC) $(CFLAGS) send_periodic.c -o send_periodic -I ../.. -I../../../common -I../../../../common -Wl,-rpath,/home/akhe/development/VSCP/vscp/src/vscp/helperlib/linux -L/home/akhe/development/VSCP/vscp/src//vscp/helperlib/linux -lvscphelper

bigtest:       bigtest.c
	$(CC) $(CFLAGS) -g -O0 bigtest.c -o bigtest -I../../../common -I../../../../common -L.. -lm -Wl,-rpath,/home/akhe/development/VSCP/vscp/src/vscp/helperlib/linux -L/home/akhe/development/VSCP/vscp/src//vscp/helperlib/linux -lvscphelper

simple_tls_client:      simple_tls_client.c simple_tls_client.h
	$(CC)  $(CFLAGS) -DNDEBUG  simple_tls_client.c -o simple_tls_client  -I../../../common -I../../../../common -Wl,-rpath,/home/akhe/development/VSCP/vscp/src/vscp/helperlib/linux -L/home/akhe/development/VSCP/vscp/src//vscp/helperlib/linux -lssl -lcrypto  -lm  

simple_tls_server:      simple_tls_server.c 
	$(CC)  $(CFLAGS)  simple_tls_server.c -o simple_tls_server  -I../../../common -I../../../../common -Wl,-rpath,/home/akhe/development/VSCP/vscp/src/vscp/helperlib/linux -L/home/akhe/development/VSCP/vscp/src//vscp/helperlib/linux -lssl -lcrypto  -lm  



# sockettcp.o

clean:
	rm -f example1
	rm -f bigtest
	rm -f stcptest
	rm -f send1
	rm -f send_periodic
	rm -f simple_tls_clinet
	rm -f simple_tls_server
