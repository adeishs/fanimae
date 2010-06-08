CC=gcc
CFLAGS=-ansi -Wall -pedantic -O3 -DNDEBUG

.PHONY: clean all

all: fnmib

fnmib: fnmib.o oakpark.o

fnmib.o: fnmib.c oakpark.h

oakpark.o: oakpark.c oakpark.h

clean:
	rm -f *.o fnmib
