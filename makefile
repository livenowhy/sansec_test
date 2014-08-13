################################################################################
# 
# Copyright(c) SWXA Corp. All rights reserved.
# 
# Makefile for swxa swsds test
# by yaahao, 2009.7.1
################################################################################

CC=gcc
CFLAGS=-Wall -D_LOG_TRACE_  -I.
LIBS = -L/usr/lib -lswsds -lpthread
OBJS=./swsds.o util.o mysansec.o
APPS=Test

all:$(APPS)

%.o:%.c
	$(CC) -O2 -c $(CFLAGS)  $< -o $@

$(APPS):$(OBJS)
	$(CC) -O2 -o $(APPS) $(OBJS) $(LIBS) 
clean:
	rm  -f *.o

cleanall:
	rm  -f *.o $(APPS)
