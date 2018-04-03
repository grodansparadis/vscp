CFLAGS = -std=c99 -g -O0

all:	example1 send1 bigtest stcptest sockettcp.o

sockettcp.o: ../../../../common/sockettcp.c ../../../../common/sockettcp.h
	$(CC) $(CFLAGS) -c ../../../../common/sockettcp.c -o $@ -I../../../common -I../../../../common

example1:	example1.c
	$(CC) $(CFLAGS) example1.c -o example1 -I ../.. -I../../../common -I../../../../common -Wl,-rpath,/home/akhe/development/VSCP/vscp/src/vscp/helperlib/linux -L/home/akhe/development/VSCP/vscp/src//vscp/helperlib/linux -lvscphelper

send1:       send1.c
	        $(CC) $(CFLAGS) send1.c -o send1 -I ../.. -I../../../common -I../../../../common -Wl,-rpath,/home/akhe/development/VSCP/vscp/src/vscp/helperlib/linux -L/home/akhe/development/VSCP/vscp/src//vscp/helperlib/linux -lvscphelper

bigtest:       bigtest.c
	$(CC) $(CFLAGS) -g -O0 bigtest.c -o bigtest -I../../../common -I../../../../common -L.. -lm -Wl,-rpath,/home/akhe/development/VSCP/vscp/src/vscp/helperlib/linux -L/home/akhe/development/VSCP/vscp/src//vscp/helperlib/linux -lvscphelper

#stcptest:      stcptest.c sockettcp.o
#	$(CC) -std=c99 $(CFLAGS)  stcptest.c -o stcptest  sockettcp.o -I../../../common -I../../../../common  -L.. -lssl -lcrypto  -lm  -lvscphelper

stcptest:      stcptest.c sockettcp.o
	$(CC)  $(CFLAGS)  stcptest.c -o stcptest  -I../../../common -I../../../../common -Wl,-rpath,/home/akhe/development/VSCP/vscp/src/vscp/helperlib/linux -L/home/akhe/development/VSCP/vscp/src//vscp/helperlib/linux -lssl -lcrypto  -lm  -lvscphelper

# sockettcp.o

clean:
	rm example1
	rm bigtest
	rm stcptest

