CC = gcc
INCLUDE = /usr/lib
LIBS = -lpthread
OBJS =

all: multi-lookup

multi-lookup: multi-lookup.c
	$(CC) -Wall -o multi-lookup multi-lookup.c $(LIBS)

clearn:
	rm -f multi-lookup
