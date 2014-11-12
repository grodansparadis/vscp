
all:	example1 bigtest


example1:	example1.c
	cc example1.c -o example1 -L.. -lvscphelper

bigtest:       bigtest.c
	cc -std=c99 bigtest.c -o bigtest -L.. -lvscphelper

clean:
	rm example1
	rm bigtest
