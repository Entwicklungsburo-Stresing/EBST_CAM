#########################################################################
#
# Makefile for lscpcie.ko
#
# Copyright 2020 Bernhard Lang, University of Geneva
# Copyright 2020 Entwicklungsbuero Stresing (http://www.stresing.de/)
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2 as
# published by the Free Software Foundation.
#
EXTRA_CFLAGS += -Duconst= $(DEBFLAGS) -D__plx_pci__
lscpcie-objs := module-main.o device.o ioctl.o file.o proc.o mmap.o \
		dma.o pcie.o registers-common.o
obj-m := lscpcie.o
PWD := $(shell pwd)

ifneq ($(KERNELRELEASE),)
	KERNELDIR ?= /lib/modules/$(KERNELRELEASE)/build

else
	KERNELDIR ?= /lib/modules/$(shell uname -r)/build
endif

all:
	make -C $(KERNELDIR) M=$(PWD) modules

clean:
	rm -f *.o *~ modules.order Module.symvers lscpcie.ko lscpcie.mod.c .*.cmd
	rm -rf .tmp_versions
	make -C $(KERNELDIR) M=$(PWD) clean