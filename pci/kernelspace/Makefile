# Makefile for LSCPCI line scan cameras IgB Stresing
#
# Copyright (C) 2010-2020 Bernhard Lang, University of Geneva
# 
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; version 2 of the License.
# 
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software Foundation,
# Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

DEBUG = n

ifeq ($(DEBUG),y)
  DEBFLAGS = -O -g -DDEBUG
else
  DEBFLAGS = -O2
endif

EXTRA_CFLAGS += $(DEBFLAGS) -D__plx_pci__

ifneq ($(KERNELRELEASE),)
  lscpci-objs := module-main.o file.o proc.o ioctl.o dma.o \
    camera.o pci-plx.o plx9056.o register-names-pci-plx.o # plx9080.o
  obj-m := lscpci.o
else
  KERNELDIR ?= /lib/modules/$(shell uname -r)/build
  PWD := $(shell pwd)

modules:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules
endif


clean:
	rm -f *.o *~ modules.order Module.symvers lscpci.ko lscpci.mod.c \
          .*.cmd
	rm -rf .tmp_versions
