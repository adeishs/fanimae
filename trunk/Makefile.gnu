CC=gcc
CFLAGS=-ansi -Wall -pedantic -O3 -DNDEBUG

.PHONY: clean all

all: fnmib fnmspioi

fnmspioi: fnmspioi.o oakpark.o

fnmib: fnmib.o oakpark.o

fnmspioi.o: fnmspioi.c oakpark.h

fnmib.o: fnmib.c oakpark.h

oakpark.o: oakpark.c oakpark.h

clean:
	rm -f *.o fnmib fnmspioi
