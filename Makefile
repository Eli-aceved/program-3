# Makefile for CPE464 tcp and udp test code
# updated by Hugh Smith - April 2023
# Modified by Elizabeth Acevedo
# all target makes UDP test code


CC= gcc
CFLAGS= -g -Wall -std=gnu99
LIBS = 

OBJS = networks.o gethostbyname.o pollLib.o safeUtil.o helperfuncs.o windowBuffer.o rcopyCBuffer.o

#uncomment next two lines if your using sendtoErr() library
LIBS += libcpe464.2.21.a -lstdc++ -ldl
CFLAGS += -D__LIBCPE464_


all: All

All: rcopy server

rcopy: rcopy.c $(OBJS) 
	$(CC) $(CFLAGS) -o rcopy rcopy.c $(OBJS) $(LIBS)

server: server.c $(OBJS) 
	$(CC) $(CFLAGS) -o server server.c  $(OBJS) $(LIBS)



.c.o:
	gcc -c $(CFLAGS) $< -o $@ $(LIBS)

cleano:
	rm -f *.o

clean:
	rm -f rcopy server *.o




