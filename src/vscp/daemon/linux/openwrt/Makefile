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

CANALSOCKETCAN_SOURCES=socketcandrv.cpp socketcanobj.cpp dllist.c devicelist.cpp
CANALSOCKETCAN_OBJECTS=$(patsubst %.c,%.o,$(patsubst %.cpp,%.o,$(CANALSOCKETCAN_SOURCES)))

OBJECTS=$(patsubst %.c,%.o,$(patsubst %.cpp,%.o,$(SOURCES)))
LDFLAGS += -s

#all: socketcandrv.so
all: $(SOURCES) $(EXECUTABLE) socketcandrv.so
#all: $(SOURCES) $(EXECUTABLE)

# Build the Linux DL
%.so: %.o $(CANALSOCKETCAN_OBJECTS)
	$(CXX) -o $@ $(CANALSOCKETCAN_OBJECTS) $(DLFLAGS) $(LIBS)

clean:
	rm -f *.o *.so $(EXECUTABLE)

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

