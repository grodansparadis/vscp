
all:	example1 bigtest


example1:	example1.c
	cc example1.c -o example1 -I ../.. -I../../../common -I../../../../common -L.. -lvscphelper

bigtest:       bigtest.c
	cc -std=c99 bigtest.c -o bigtest -I../../../common -I../../../../common -L.. -lvscphelper

clean:
	rm example1
	rm bigtest
