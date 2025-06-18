# $Id: Makefile,v 1.4 2008/02/28 18:08:40 lav Exp $
#
# Makefile for Aladdin Linux Parallel Driver
# ------------------------------------------
#
# Copyright (c) 2005 by Aladdin Knowledge Systems Ltd.
#

include $(here)linuxdefs.mk

TARGETARCH := $(shell uname -m | sed -e s/i.86/x86/)

KERNSRC ?= /lib/modules/$(shell uname -r)/build

obj-m       := aksparlnx.o
aksparlnx-y := aksparpub.o api.o
#aksparlnx-objs := aksparpub.o api.o

CC            = gcc

MODULE_DEFINES := -D__KERNEL__ -DMODULE
HLMOD_DEFINES  := -DHL_MAJOR=42 -DCPC_PCI

#OUR_CFLAGS        = $(MODULE_DEFINES) $(HLMOD_DEFINES)
OUR_CFLAGS        = $(HLMOD_DEFINES)

CFLAGS.x86    = $(LINUX_COPTS) $(LINUX_I386_OPTS) $(LINUX_I386_KERN_OPTS)
CFLAGS.alpha  = $(LINUX_COPTS) $(LINUX_ALPHA_OPTS) $(LINUX_ALPHA_KERN_OPTS)
CFLAGS.ia64   = $(LINUX_COPTS) $(LINUX_IA64_OPTS) $(LINUX_IA64_KERN_OPTS)
CFLAGS.ppc    = $(LINUX_COPTS) $(LINUX_PPC_OPTS) $(LINUX_PPC_KERN_OPTS)
CFLAGS.x86_64 = $(LINUX_COPTS) $(LINUX_AMD64_OPTS) $(LINUX_AMD64_KERN_OPTS)

OUR_CFLAGS   += $(CFLAGS.$(TARGETARCH))
#EXTRA_CFLAGS := $(MODULE_DEFINES) $(HLMOD_DEFINES)
EXTRA_CFLAGS := $(HLMOD_DEFINES)

default: api.o
	touch .api.o.cmd
	$(MAKE) V=1 -C $(KERNSRC) here=$$(pwd)/ SUBDIRS=$$(pwd) M=$(PWD) modules
	strip --strip-debug --discard-all aksparlnx.ko

api.o: api.o_shipped.x86 api.o_shipped.x86_64
	cp api.o_shipped.$(TARGETARCH) $@

-include $(KERNSRC)/Rules.make

clean:
	rm -rf .tmp_versions
	rm -f .a*.* *.o *.ko aksparlnx.mod.c *.mod *.order *.symvers .*.cmd

help:
	@echo "usage:	$(MAKE)"
	@echo "		build module"
	@echo "	$(MAKE) clean"
	@echo "		clean up"

# Local Variables: #
# mode: makefile #
# End: #
