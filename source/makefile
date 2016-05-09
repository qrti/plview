# makefile
# part of plview
#
# published under the terms of BSD 2-clause license
# Copyright (c) 2016, qrt@qland.de 
# All rights reserved

#  -g    adds debugging information to the executable file
#  -Wall turns on most, but not all, compiler warnings

CC = gcc
LIBS = -lm -pthread
CFLAGS = -Wall
CFLAGS2 = -Wall -std=c99

default: plview

plview: plview.o ssdp.o socket.o mem.o common.o json.o  gc.o display.o ini.o
	$(CC) $(CFLAGS) $(LIBS) -o plview plview.o ssdp.o socket.o mem.o common.o json.o  gc.o display.o ini.o

plview.o: plview.c
	$(CC) $(CFLAGS) -c plview.c

ssdp.o: ssdp.c ssdp.h
	$(CC) $(CFLAGS) -c ssdp.c
	
socket.o: socket.c socket.h
	$(CC) $(CFLAGS) -c socket.c	

mem.o: mem.c mem.h
	$(CC) $(CFLAGS) -c mem.c
		
common.o: common.c common.h
	$(CC) $(CFLAGS) -c common.c			
	
json.o: json.c json.h
	$(CC) $(CFLAGS) -c json.c			
	
gc.o: gc.c gc.h
	$(CC) $(CFLAGS) -c gc.c	

display.o: display.c display.h
	$(CC) $(CFLAGS2) -c display.c	

ini.o: ini.c ini.h
	$(CC) $(CFLAGS2) -c ini.c	

clean: 
	$(RM) tosh *.o *~