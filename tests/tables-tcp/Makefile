
all:	test_tables_tcp


test_tables_tcp:       test_tables_tcp.c
	cc -std=c99 test_tables_tcp.c -o test_tables_tcp -I../../src/common -I../../src/vscp/common -L../../src/vscp/helperlib/linux -lm -lvscphelper

clean:
	rm -f test_helperlib
	rm -f *.o
