
all:	test_helperlib


test_helperlib:       test_helperlib.c
	cc -std=c99 -UWXDEBUG test_helperlib.c -o test_helperlib -I../../src/common -I../../src/vscp/common -L../../src/vscp/helperlib/linux -lm -lvscphelper

clean:
	rm -f test_helperlib
	rm -f *.o
