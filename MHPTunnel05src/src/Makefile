### Makefile.bcc
OBJS = tunnel-client.obj tunnel-server.obj tunnel-common.obj
PROG = tunnel-client tunnel-server
TDS  = tunnel-client.tds tunnel-server.tds tunnel-common.tds
EXE  = $(PROG).exe
LIB  = wpcap.lib ws2_32.lib

.SUFFIXES : .obj .c .cpp .h
.SUFFIXES : .res .rc

ALL: tunnel-client.exe tunnel-server.exe

clean:
	del $(OBJS) $(EXE) $(TDS)

tunnel-client.exe: tunnel-client.cpp tunnel-common.cpp tunnel-common.h
	$(CC) -L $(LIB) tunnel-client.cpp tunnel-common.cpp

tunnel-server.exe: tunnel-server.cpp tunnel-common.cpp tunnel-common.h
	$(CC) -L $(LIB) tunnel-server.cpp tunnel-common.cpp
