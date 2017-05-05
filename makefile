# Time-stamp: <02/11/09 10:40:24 INGA>
# File: Makefile

COMPILE=g++
CC=gcc
LINK=g++
GIT=git
CARG=-c -O4 -Wall -DVERSION=\"$(GIT_VERSION)\"

DESTDIR=/
PREFIX=$(DESTDIR)/usr/local
DEPDIR=.deps


OBJECTa= udpclient.o rnd.o
OBJECTb= udpserver.o sample.o
OBJECTb1= udpecho.o
OBJECTc= tcpclient.o rnd.o
OBJECTd= tcpserver.o sample.o
OBJECTe= icmpclient.o rnd.o
OBJECT= server.o sample.o version.o

GIT_VERSION := $(shell git describe --abbrev=4 --dirty --always --tags)


targeta=udpclient
targetb=udpserver
targetb1=udpecho
targetc=tcpclient
targetd=tcpserver
targete=icmpclient
target=server


all: $(OBJECTa)	$(OBJECTb) $(OBJECTb1) $(OBJECTc) $(OBJECTd) $(OBJECTe) $(OBJECT)
	$(COMPILE) -o $(targeta) $(OBJECTa)
	$(COMPILE) -o $(targetb) $(OBJECTb)
	$(CC) -o $(targetb1) $(OBJECTb1)
	$(COMPILE) -o $(targetc) $(OBJECTc)
	$(COMPILE) -o $(targetd) $(OBJECTd)
	$(COMPILE) -o $(targete) $(OBJECTe)
	$(COMPILE) -o $(target) $(OBJECT)

clean:
	rm -f *.o *.exe
	rm -r $(targeta) $(targetb) $(targetb1) $(targetc) $(targetd) $(targete) $(target)

install: 
	install -m 0755 udpclient $(PREFIX)/bin
	install -m 0755 udpserver $(PREFIX)/bin
	install -m 0755 udpecho $(PREFIX)/bin
	install -m 0755 tcpclient $(PREFIX)/bin
	install -m 0755 tcpserver $(PREFIX)/bin
	install -m 0755 icmpclient $(PREFIX)/bin
	install -m 0755 server $(PREFIX)/bin



server.o: server.cpp
	$(COMPILE) $(CARG) server.cpp

udpserver.o: udpserver.cpp
	$(COMPILE) $(CARG) udpserver.cpp

udpecho.o: udpecho.c
	$(CC) $(CARG) udpecho.c

udpclient.o: udpclient.cpp
	$(COMPILE) $(CARG) udpclient.cpp

sample.o: sample.cpp sample.h
	$(COMPILE) $(CARG) sample.cpp

rnd.o:      rnd.cpp rnd.h makefile
	$(COMPILE) $(CARG) rnd.cpp

tcpserver.o: tcpserver.cpp
	$(COMPILE) $(CARG) tcpserver.cpp

tcpclient.o: tcpclient.cpp
	$(COMPILE) $(CARG) tcpclient.cpp

icmpclient.o: icmpclient.cpp
	$(COMPILE) $(CARG) icmpclient.cpp

version.o: .FORCE
	$(GIT) rev-parse HEAD | awk ' BEGIN {print "#include \"version.h\""} {print "const char * build_git_sha = \"" $$0"\";"} END {}' > version.c
	date | awk 'BEGIN {} {print "const char * build_git_time = \""$$0"\";"} END {} ' >> version.c
	$(COMPILE) $(CARG) version.c

version.c: .FORCE 
	$(GIT) rev-parse HEAD | awk ' BEGIN {print "#include \"version.h\""} {print "const char * build_git_sha = \"" $$0"\";"} END {}' > version.c
	date | awk 'BEGIN {} {print "const char * build_git_time = \""$$0"\";"} END {} ' >> version.c

.FORCE: 
