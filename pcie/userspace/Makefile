# Makefile for lscpci ccd cameras igb stresing
#
# Copyright (C) 2020 Bernhard Lang, University of Geneva
# Copyright 2020 Entwicklungsbuero Stresing (http://www.stresing.de/)
# 
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2 as
# published by the Free Software Foundation.

CFLAGS = -Wall -g -fpic -D__user_space__

tests = test-mmap test-read-write test-register test-readout-polling

library: liblscpcie.so.1.0.0

tests: $(tests)

debug: camera-debug

all: library tests debug stop-camera links

liblscpcie.so.1.0.0: lscpcie.o
	gcc $(CFLAGS) $^ -shared -Wl,-soname,liblscpcie.so.1 -o $@

links: liblscpcie.so.1.0 liblscpcie.so.1 liblscpcie.so

liblscpcie.so.1.0:
	ln -s $@.0 $@

liblscpcie.so.1:
	ln -s $@.0.0 $@

liblscpcie.so:
	ln -s $@.1.0.0 $@

lscpcie.o: lscpcie.h ../kernelspace/ioctl.h

%.o: %.c
	gcc $(CFLAGS) -o $@ -c $<

test-mmap: test-mmap.o lscpcie.o
	gcc -Wall -o $@ $^

test-read-write: test-read-write.o lscpcie.o
	gcc -Wall -o $@ $^

test-register: test-register.o lscpcie.o
	gcc -Wall -o $@ $^

test-readout-polling: test-readout-polling.o lscpcie.o
	gcc -Wall -o $@ $^

camera-debug: camera-debug.o lscpcie.o
	gcc -Wall -o $@ $^

stop-camera: stop-camera.o lscpcie.o
	gcc -Wall -o $@ $^

clean:
	rm -f *.o *~ $(tests) liblscpcie.so.1.0.0
