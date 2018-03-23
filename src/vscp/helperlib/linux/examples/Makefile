
all:	example1 bigtest stcptest sockettcp.o

sockettcp.o: ../../../../common/sockettcp.c ../../../../common/sockettcp.h
	$(CC) $(CFLAGS) -c ../../../../common/sockettcp.c -o $@ -I../../../common -I../../../../common

example1:	example1.c
	cc example1.c -o example1 -I ../.. -I../../../common -I../../../../common -L.. -lvscphelper

bigtest:       bigtest.c
	cc -std=c99 bigtest.c -o bigtest -I../../../common -I../../../../common -L.. -lm -lvscphelper

stcptest:      stcptest.c sockettcp.o
	cc -std=c99 stcptest.c -o stcptest sockettcp.o -I../../../common -I../../../../common -L.. -lssl -lcrypto  -lm -lvscphelper



clean:
	rm example1
	rm bigtest
	rm stcptest

