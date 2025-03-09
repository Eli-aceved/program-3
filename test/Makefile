CC= gcc
CFLAGS= -g -Wall
LIBS = 

OBJS = rcopyCBuffer.o

all: rcopyCBuffer

rcopyCBuffer: rcopyCBuffer.c $(OBJS) 
	$(CC) $(CFLAGS) -o rcopyCBuffer $(OBJS) $(LIBS)

.c.o:
	gcc -c $(CFLAGS) $< -o $@ $(LIBS)

cleano:
	rm -f *.o

clean:
	rm -f rcopyBuffer *.o
