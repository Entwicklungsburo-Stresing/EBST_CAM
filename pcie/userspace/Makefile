# Makefile for lscpcie line scan and area cameras igb stresing
#
# Copyright (C) 2020-2021 Bernhard Lang, University of Geneva
# Copyright 2020-2021 Entwicklungsbuero Stresing (http://www.stresing.de/)
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2 as
# published by the Free Software Foundation.

CFLAGS = -Wall -g -fpic -D__user_space__

version = 1
subversion = 0
patchlevel = 0

tests = test-mmap test-read-write test-register

library = liblscpcie.so.$(version).$(subversion).$(patchlevel)
links = liblscpcie.so liblscpcie.so.$(version) \
  liblscpcie.so.$(version).$(subversion)

library: $(library)

tests: $(tests)

.PHONY: examples
examples:
	$(MAKE) -C examples

debug: camera-debug

all: library tests debug stop-camera links examples

$(library): lscpcie.o
	gcc $(CFLAGS) $^ -shared -Wl,-soname,liblscpcie.so.$(version) -o $@

links: $(links)

$(links): $(library)
	ln -s $< $@

lscpcie.o: lscpcie.h ../kernelspace/ioctl.h

%.o: %.c
	gcc $(CFLAGS) -o $@ -c $<

test-mmap: test-mmap.o lscpcie.o
	gcc -Wall -o $@ $^

test-read-write: test-read-write.o lscpcie.o
	gcc -Wall -o $@ $^

test-register: test-register.o lscpcie.o
	gcc -Wall -o $@ $^

camera-debug: camera-debug.o lscpcie.o
	gcc -Wall -o $@ $^

stop-camera: stop-camera.o lscpcie.o
	gcc -Wall -o $@ $^

clean:
	rm -f *.o *~

distclean: clean
	rm -f $(tests) $(links) \
          liblscpcie.so.$(version).$(subversion).$(patchlevel)
	$(MAKE) -C examples distclean
