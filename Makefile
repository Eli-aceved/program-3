# Makefile 
# written by Hugh Smith - April 2019

CC= gcc
CFLAGS= -g -Wall -std=gnu99
LIBS = 

OBJS = circularbuffer.o 

all:   circularbuffer

buffer: circularbuffer.c $(OBJS)
	$(CC) $(CFLAGS) -o circularbuffer circularbuffer.c $(OBJS) $(LIBS)

.c.o:
	gcc -c $(CFLAGS) $< -o $@ $(LIBS)

cleano:
	rm -f *.o

clean:
	rm -f circularbuffer *.o

