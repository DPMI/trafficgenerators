# Time-stamp: <02/11/09 10:40:24 INGA>
# File: Makefile
# LinkAggregator
#clients
#g++ -c udpClient1.cpp rnd.cpp
#g++ -o udpclientv udpClient1.o rnd.o

#g++ -c udpClient2.cpp rnd.cpp
#g++ -o udpclientv2 udpClient2.o rnd.o


#g++ -c tcpClient.cpp rnd.cpp
#g++ -o tcpclientv tcpClient.o rnd.o 

#g++ -c tcpClient2.cpp rnd.cpp
#g++ -o tcpclientv2 tcpClient.o rnd.o 

#g++ -c icmpv.cpp rnd.cpp
#g++ -o icmpclientv icmp.o rnd.o 

#g++ -c icmpv2.cpp rnd.cpp
#g++ -o icmpclientv2 icmp.o rnd.o 


#servers
#g++ -c udpServer1.cpp sample.cpp
#g++ -o udpserverv udpServer1.o sample.o

#g++ -c tcpServer.cpp sample.cpp 
#g++ -o tcpserverv tcpServer.o sample.o 
COMPILE=g++
LINK=g++
CARG=-c -O4 -Wall 
OBJECTd= udpServer1.o sample.o 
OBJECTc= udpClient1.o rnd.o
OBJECTi= udpClient2.o rnd.o
OBJECTl= udpClient3.o rnd.o
OBJECTe= tcpClient.o rnd.o
OBJECTj= tcpClient2.o rnd.o
OBJECTf= tcpServer.o sample.o
OBJECTg= icmpv.o rnd.o
OBJECTh= icmpround.o rnd.o
OBJECTk= icmpv2.o rnd.o

targetd=udpserverv
targetc=udpclientv
targeti=udpclientv2
targetl=udpclientv3
targete=tcpclientv
targetj=tcpclientv2
targetf=tcpserverv
targetg=icmpclientv
targeth=icmpclientrv
targetk=icmpclientv2

#all: $(OBJECTd) $(OBJECTc) $(OBJECTe) $(OBJECTf) $(OBJECTg)	
	#$(COMPILE) -o $(targetd) $(OBJECTd)
	#$(COMPILE) -o $(targetc) $(OBJECTc)  $(COMPILE) -o $(targete) $(OBJECTe)	
	#$(COMPILE) -o $(targetf) $(OBJECTf) $(COMPILE) -o $(targetg) $(OBJECTg)
	#$(COMPILE) -o $(targete) $(OBJECTe)		
	#$(COMPILE) -o $(targetf) $(OBJECTf)
	#$(COMPILE) -o $(targetg) $(OBJECTg)

all: $(OBJECTd)	$(OBJECTc) $(OBJECTi) $(OBJECTe) $(OBJECTj) $(OBJECTf) $(OBJECTg) $(OBJECTh) $(OBJECTk) $(OBJECTl)
	$(COMPILE) -o $(targetd) $(OBJECTd)
	$(COMPILE) -o $(targetc) $(OBJECTc)
	$(COMPILE) -o $(targeti) $(OBJECTi)
	$(COMPILE) -o $(targete) $(OBJECTe)
	$(COMPILE) -o $(targetj) $(OBJECTj)
	$(COMPILE) -o $(targetf) $(OBJECTf)
	$(COMPILE) -o $(targetg) $(OBJECTg)
	$(COMPILE) -o $(targeth) $(OBJECTh)
	$(COMPILE) -o $(targetk) $(OBJECTk)	
	$(COMPILE) -o $(targetl) $(OBJECTl)

clean:
	rm -f *.o *.exe
	rm -r $(OBJECTd) $(OBJECTc) $(OBJECTe) $(OBJECTf) $(OBJECTg) $(OBJECTh) $(OBJECTk) $(OBJECTi) 

udpServer1.o: udpServer1.cpp
	$(COMPILE) $(CARG) udpServer1.cpp

udpClient1.o: udpClient1.cpp
	$(COMPILE) $(CARG) udpClient1.cpp

udpClient2.o: udpClient2.cpp
	$(COMPILE) $(CARG) udpClient2.cpp

udpClient3.o: udpClient3.cpp
	$(COMPILE) $(CARG) udpClient3.cpp

sample.o: sample.cpp sample.h
	$(COMPILE) $(CARG) sample.cpp

rnd.o:      rnd.cpp rnd.h makefile
	$(COMPILE) $(CARG) rnd.cpp

tcpServer.o: tcpServer.cpp
	$(COMPILE) $(CARG) tcpServer.cpp

tcpClient.o: tcpClient.cpp
	$(COMPILE) $(CARG) tcpClient.cpp

tcpClient2.o: tcpClient2.cpp
	$(COMPILE) $(CARG) tcpClient2.cpp

icmpv.o: icmpv.cpp
	$(COMPILE) $(CARG) icmpv.cpp

icmpround.o:	icmpround.cpp
	$(COMPILE) $(CARG) icmpround.cpp

icmpv2.o: icmpv2.cpp
	$(COMPILE) $(CARG) icmpv2.cpp



