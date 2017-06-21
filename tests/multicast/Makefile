
CC = `wx-config --cc`
CXX = `wx-config --cxx`
CFLAGS = -std=c99 -g -O0 -D_BSD_SOURCE  -I../../src/vscp/common -I../../src/common
CXXFLAGS = -g -O0
CPPFLAGS =  -I../../src/vscp/common -I../../src/common
LDFLAGS =
EXTRALIBS =  -lz
OBJ = crc.o crc8.o aes.o

all: sender alistener mclistener

crc.o: ../../src/common/crc.c ../../src/common/crc.h
	$(CC) $(CFLAGS) -c ../../src/common/crc.c -o $@

crc8.o: ../../src/common/crc8.c ../../src/common/crc8.h
	$(CC) $(CFLAGS) -c ../../src/common/crc8.c -o $@

aes.o: ../../src/common/aes.c ../../src/common/aes.h
	$(CC) $(CFLAGS) -c ../../src/common/aes.c -o $@

sender: $(OBJ) sender.c
	$(CC) $(CFLAGS) sender.c -o $@ $(OBJ)

alistener:	$(OBJ) alistener.c
	$(CC) $(CFLAGS) alistener.c -o $@ $(OBJ)

mclistener:	$(OBJ) mclistener.c
	$(CC) $(CFLAGS) mclistener.c -o $@ $(OBJ)

clean:
	rm -f alistener
	rm -f mclistener
	rm -f sender
	rm -f *.o

$(ALWAYS_BUILD):  .FORCE

.FORCE:

# Include dependency info, if present:
-include .deps/*.d

.PHONY: all install uninstall clean distclean data .FORCE
