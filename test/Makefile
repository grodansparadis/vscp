
all:	helperlib


example1:	example1.c
	cc example1.c -o example1 -I ../.. -I../../../common -I../../../../common -L.. -lvscphelper

helperlib:       helperlib.c
	cc -std=c99 helperlib.c -o helperlib -I../src/vscp/common -I../src/vscp/common -L../src/vscp/helperlib/linux -lm -lvscphelper

clean:
	rm example1
	rm bigtest
