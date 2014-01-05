EXECUTABLE=evscpd
SOURCES=vscpd.cpp \
	daemonvscp.cpp \
	clientlist.cpp \
	controlobject.cpp \
	devicelist.cpp \
	vscphelper.cpp \
	tcpipclientthread.cpp \
	dm.cpp \
	vscpvariable.cpp \
	interfacelist.cpp \
	userlist.cpp \
	udpthread.cpp \
	devicethread.cpp \
	vscptcpif.cpp \
	guid.cpp \
	dllist.c \
	configfile.cpp \
	crc.c \
	md5.cpp \
	randPassword.cpp

OBJECTS=$(patsubst %.c,%.o,$(patsubst %.cpp,%.o,$(SOURCES)))
LDFLAGS += -s

all: $(SOURCES) $(EXECUTABLE)
    
clean:
	rm -f *.o $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
ifeq (,$(findstring .cpp,$(SOURCES)))
	$(CC) $(LDFLAGS) $(OBJECTS) $(LIBS) -o $@
else
	$(CXX) $(LDFLAGS) $(OBJECTS) $(LIBS) -o $@
endif

.c.o:
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

.cpp.o:
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

