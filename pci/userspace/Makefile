# Makefile for lscpci ccd cameras igb stresing
#
# Copyright (C) 2010-2017 Bernhard Lang, University of Geneva
# 
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or (at
# your option) any later version.
# 
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software Foundation,
# Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

CFLAGS = -Wall -g -fpic
programs = test-camera camera-dump camera-register-play camera-debug stop-camera

test-camera-objects = test-camera.o lscpci.o ../kernelspace/register-names.o
stop-camera-objects = stop-camera.o lscpci.o
camera-dump-objects = camera-dump.o lscpci.o ../kernelspace/register-names.o
camera-register-play-objects = camera-register-play.o lscpci.o \
  ../kernelspace/register-names.o
camera-debug-objects = camera-debug.o lscpci.o

all: $(programs) liblscpci.so.1.0.0

liblscpci.so.1.0.0: lscpci.o
	gcc $(CFLAGS) $^ -shared -Wl,-soname,liblscpci.so.1 -o $@

test-camera: $(test-camera-objects)
	gcc $(CFLAGS) -o $@ $(test-camera-objects)

stop-camera: $(stop-camera-objects)
	gcc $(CFLAGS) -o $@ $(stop-camera-objects)

camera-dump: $(camera-dump-objects)
	gcc $(CFLAGS) -o $@ $(camera-dump-objects)

camera-register-play: $(camera-register-play-objects)
	gcc $(CFLAGS) -o $@ $(camera-register-play-objects)

camera-debug: $(camera-debug-objects)
	gcc $(CFLAGS) -o $@ $(camera-debug-objects)

lscpci.o: lscpci.h ../kernelspace/ioctl.h

camera-idle.o: lscpci.h

%.o: %.c
	gcc $(CFLAGS) -o $@ -c $<


clean:
	rm -f *.o *~

distclean: clean
	rm -f $(programs) liblscpci.so liblscpci.so.*
