
all:	test_helperlib


example1:	example1.c
	cc example1.c -o example1 -I ../.. -I../../../common -I../../../../common -L.. -lvscphelper

test_helperlib:       test_helperlib.c
	cc -std=c99 test_helperlib.c -o test_helperlib -I../src/common -I../src/vscp/common -L../src/vscp/helperlib/linux -lm -lvscphelper

clean:
	rm example1
	rm test_helperlib
