
# the list of files to produce the library 
OBJS = child1.o

LD=ld
CPP=g++
CC=gcc
CFLAGS=-O
LDFLAGS = --noinhibit-exec
DLLTOOL= dlltool --as=as

# Without any options ( b17.1 + win95 ) 
# test2 NOK 
# test2 OK with ``LDFLAGS-libchild1 = --image-base=0x11000000'' only  

LDFLAGS-libchild1 = --image-base=0x11000000 

all: libchild1.dll libchild1.stubs.a  test2 

libchild1.a: $(OBJS)
	rm -f libchild1.a
	ar cr libchild1.a $(OBJS)
	ranlib libchild1.a

include Makefile.DLLs

# test Version with link at startup 

# Version with LoadLibrary 
# 

test2	: test2.cpp  libchild1.a 
	$(CPP) $(CFLAGS) $(LINKEROPT)  -o test2.exe test2.cxx 


clean	:
	rm -f libchild1.a libchild1.dll libchild1.stubs.a lib_name libs_name
	rm -f test2.exe *.o libchild1.def libchild1.exp libchild1.base
	rm -f libs_path child1.base *~ *.*~ *.s

glue.o: glue.c
	$(CC) -c -o $@ $< 

%.o: %.cxx
	$(CXX) -c -o $@ $< 

distclean: clean
