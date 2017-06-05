

CC = `wx-config --cc`
CXX = `wx-config --cxx`
MONGOOSE_CFLAGS = -DMG_ENABLE_THREADS -DMG_ENABLE_GETADDRINFO -UMG_ENABLE_SSL
CFLAGS = -g -O0 -pthread -D MG_ENABLE_THREADS -I../../src/vscp/common -I../../src/common
CXXFLAGS = -g -O0 
CPPFLAGS =  -I../../src/vscp/common -I../../src/common 
CPPFLAGS += `wx-config --cxxflags`
CPPFLAGS += $(MONGOOSE_CFLAGS)
LDFLAGS = 
EXTRALIBS =  -lz -lrt -lm -lcrypto -lpthread
EXTRALIBS += `wx-config --libs all`
TEST_OBJECTS =  test_tables_remote_class.o \

TEST_SPECIALS = vscphelper.o \
	vscpremotetcpif.o \
	guid.o \
	mongoose.o \
	crc8.o \
	crc.o \



all:	test_tables_remote_class

vscphelper.o: ../..//src/vscp/common/vscphelper.o ../../src/vscp/common/vscphelper.h
	$(CXX)  $(CXXFLAGS) $(CPPFLAGS) -c ../../src/vscp/common/vscphelper.cpp -o $@

vscpremotetcpif.o: ../../src/vscp/common/vscpremotetcpif.o ../../src/vscp/common/vscpremotetcpif.h
	$(CXX)  $(CXXFLAGS) $(CPPFLAGS) -c ../../src/vscp/common/vscpremotetcpif.cpp -o $@

guid.o: ../../src/vscp/common/guid.o ../../src/vscp/common/guid.h
	$(CXX)  $(CXXFLAGS) $(CPPFLAGS) -c ../../src/vscp/common/guid.cpp -o $@

crc.o: ../../src/common/crc.o ../../src/common/crc.h 
	$(CC) $(CFLAGS) -c ../../src/common/crc.c -o $@

crc8.o: ../../src/common/crc8.o ../../src/common/crc8.h
	$(CC) $(CFLAGS) -c ../../src/common/crc8.c -o $@	

mongoose.o: ../../src/common/mongoose.o ../../src/common/mongoose.h
	$(CC) $(CFLAGS) $(MONGOOSE_CFLAGS) -c ../../src/common/mongoose.c -o $@ 	

test_tables_remote_class: $(TEST_OBJECTS) $(TEST_SPECIALS)
	$(CXX)  -o $@ $(TEST_OBJECTS) $(TEST_SPECIALS) $(LDFLAGS) -L$(LIBDIRNAME)  $(EXTRALIBS)

clean:
	rm -f test_tables_remote_class
	rm -f *.o
