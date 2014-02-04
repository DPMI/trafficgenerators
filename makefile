# Time-stamp: <02/11/09 10:40:24 INGA>
# File: Makefile
# LinkAggregator

.DEFAULT_GOAL := all 
COMPILE=g++
COMPILE2=gcc
LINK=g++
LINK2=gcc
CARG= -c -O4 -Wall 
CARG2= -c -O4 
OBJECTd= udpServer1.o sample.o 
OBJECTc= udpClient1.o 
OBJECTa= tcpServer.o sample.o 
OBJECTb= tcpClient.o 
OBJECTe= anping.o

targetd=udpServer1
targetc=udpClient1

targeta=tcpServer
targetb=tcpClient

targete=anping



ping: $(OBJECTe)
	$(COMPILE2) -o $(targete) $(OBJECTe)

udp: $(OBJECTd) $(OBJECTc)
	$(COMPILE) -o $(targetd) $(OBJECTd)
	$(COMPILE) -o $(targetc) $(OBJECTc)

tcp: $(OBJECTa) $(OBJECTb)
	$(COMPILE) -o $(targeta) $(OBJECTa)
	$(COMPILE) -o $(targetb) $(OBJECTb)

all: $(OBJECTd) $(OBJECTc) $(OBJECTa) $(OBJECTb) $(OBJECTe)
	$(COMPILE) -o $(targetd) $(OBJECTd)
	$(COMPILE) -o $(targetc) $(OBJECTc)
	$(COMPILE) -o $(targeta) $(OBJECTa)
	$(COMPILE) -o $(targetb) $(OBJECTb)
	$(COMPILE) -o $(targete) $(OBJECTe)

clean:
	rm -f *.o *.exe


udpServer1.o: udpServer1.cpp
	$(COMPILE) $(CARG) udpServer1.cpp


udpClient1.o: udpClient1.cpp
	$(COMPILE) $(CARG) udpClient1.cpp

tcpClient.o: tcpClient.cpp
	$(COMPILE) $(CARG) tcpClient.cpp

tcpServer.o: tcpServer.cpp
	$(COMPILE) $(CARG) tcpServer.cpp

anping.o: anping.c
	$(COMPILE2) $(CARG2) anping.c

sample.o: sample.cpp sample.h
	$(COMPILE) $(CARG) sample.cpp
