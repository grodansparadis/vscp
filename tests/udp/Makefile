
all:	uc udpsrv


uc:	udpclient.c
	cc -std=c99 -D_BSD_SOURCE  udpclient.c -o uc 

udpsrv:     udpsrv.c
	cc -std=c99 udpsrv.c -o udpsrv

clean:
	rm -f uc
	rm -f udpsrv
	rm -f *.o
