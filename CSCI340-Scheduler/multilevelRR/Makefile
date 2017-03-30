# Makefile for the MultilevelRR


VERSION = 1
sim = ./sim
CC = gcc
CXX = g++
CFLAGS = -Wall -O -g
FILES = $(sim)

all: $(FILES)

sim: sim.o schedule.o
	$(CC) $(CFLAGS) -o sim sim.o schedule.o

clean:
	rm -f $(FILES) *.o
