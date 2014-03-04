# Time-stamp: <02/11/09 10:40:24 INGA>
# File: Makefile

COMPILE=g++
LINK=g++
CARG=-c -O4 -Wall 
OBJECTa= udpClient.o rnd.o
OBJECTb= udpServer.o sample.o 
OBJECTc= tcpClient.o rnd.o
OBJECTd= tcpServer.o sample.o
OBJECTe= icmpClient.o rnd.o


targeta=udpclient
targetb=udpserver
targetc=tcpclient
targetd=tcpserver
targete=icmpclient



all: $(OBJECTa)	$(OBJECTb) $(OBJECTc) $(OBJECTd) $(OBJECTe)
	$(COMPILE) -o $(targeta) $(OBJECTa)
	$(COMPILE) -o $(targetb) $(OBJECTb)
	$(COMPILE) -o $(targetc) $(OBJECTc)
	$(COMPILE) -o $(targetd) $(OBJECTd)
	$(COMPILE) -o $(targete) $(OBJECTe)

clean:
	rm -f *.o *.exe
	rm -r $(targeta) $(targetb) $(targetc) $(targetd) $(targete)

udpServer.o: udpServer.cpp
	$(COMPILE) $(CARG) udpServer.cpp

udpClient.o: udpClient.cpp
	$(COMPILE) $(CARG) udpClient.cpp

sample.o: sample.cpp sample.h
	$(COMPILE) $(CARG) sample.cpp

rnd.o:      rnd.cpp rnd.h makefile
	$(COMPILE) $(CARG) rnd.cpp

tcpServer.o: tcpServer.cpp
	$(COMPILE) $(CARG) tcpServer.cpp

tcpClient.o: tcpClient.cpp
	$(COMPILE) $(CARG) tcpClient.cpp

icmpClient.o: icmpClient.cpp
	$(COMPILE) $(CARG) icmpClient.cpp

