CC = gcc
CFLAGS = -c -g -Wall -Wextra
LFLAGS = -Wall -Wextra

.PHONY: all clean

all: lookup multi-lookup

multi-lookup: multi-lookup.o util.o
	$(CC) $(LFLAGS) $^ -o $@

lookup: lookup.o queue.o util.o
	$(CC) $(LFLAGS) $^ -o $@

multi-lookup.o: multi-lookup.c multi-lookup.h
	$(CC) $(CFLAGS) $<

lookup.o: lookup.c
	$(CC) $(CFLAGS) $<

util.o: util.c util.h
	$(CC) $(CFLAGS) $<

clean:
	rm -f lookup multi-lookup
	rm -f *.o
	rm -f *~
	rm -f results.txt
