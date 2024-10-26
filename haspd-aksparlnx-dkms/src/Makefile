# $Id: Makefile,v 1.4 2008/02/28 18:08:40 lav Exp $
#
# Makefile for Aladdin Linux Parallel Driver
# ------------------------------------------
#
# Copyright (c) 2005 by Aladdin Knowledge Systems Ltd.
#

include $(here)linuxdefs.mk

TARGETARCH := $(shell uname -m | sed -e s/i.86/x86/)

HLMOD_DEFINES  := -DHL_MAJOR=42 -DCPC_PCI

CC            = gcc

OUR_CFLAGS        = $(MODULE_DEFINES) $(HLMOD_DEFINES)

MODULE_DEFINES := -D__KERNEL__ -DMODULE

CFLAGS.x86    = $(LINUX_COPTS) $(LINUX_I386_OPTS) $(LINUX_I386_KERN_OPTS)
CFLAGS.alpha  = $(LINUX_COPTS) $(LINUX_ALPHA_OPTS) $(LINUX_ALPHA_KERN_OPTS)
CFLAGS.ia64   = $(LINUX_COPTS) $(LINUX_IA64_OPTS) $(LINUX_IA64_KERN_OPTS)
CFLAGS.ppc    = $(LINUX_COPTS) $(LINUX_PPC_OPTS) $(LINUX_PPC_KERN_OPTS)
CFLAGS.x86_64 = $(LINUX_COPTS) $(LINUX_AMD64_OPTS) $(LINUX_AMD64_KERN_OPTS)

OUR_CFLAGS        += $(CFLAGS.$(TARGETARCH))

help:
	@echo "usage:	$(MAKE) KERNSRC=kernelsourcedir kernel3"
	@echo "		build 3.x version"
	@echo "	$(MAKE) KERNSRC=kernelsourcedir kernel26"
	@echo "		build 2.6.x version"
	@echo "	$(MAKE) KERNSRC=kernelsourcedir kernel24"
	@echo "		build 2.4.x version"
	@echo "	$(MAKE) clean"
	@echo "		clean up"


KERNSRC ?= /lib/modules/$(shell uname -r)/build

obj-m        = aksparlnx.o

aksparlnx-objs     =  aksparpub.o api.o

# ------------------------------ 3.x --------------------------------

kernel3: kernel26

# ------------------------------ 2.6.x ------------------------------

EXTRA_CFLAGS := $(MODULE_DEFINES) $(HLMOD_DEFINES)

kernel26: api.o
	touch .api.o.cmd
	$(MAKE) V=1 -C $(KERNSRC) here=$$(pwd)/ SUBDIRS=$$(pwd) M=$(PWD) modules
	strip --strip-debug --discard-all aksparlnx.ko

# ------------------------------ 2.4.x ------------------------------

bymyself: aksparlnx.o

aksparlnx.o: api24.o aksparpub.o
	ld -r -x -o $@ api24.o aksparpub.o

aksparpub.o: aksparpub.h aksport.h akstypes.h fastapi.h unix_ioctl.h aksparpub.c
	$(CC) $(OUR_CFLAGS) -I $(KERNSRC)/include -c aksparpub.c -o $@

kernel24:
	$(MAKE) V=1 -C $(KERNSRC) here=$$(pwd)/ SUBDIRS=$$(pwd) modules
	strip --strip-debug --discard-all aksparlnx.o

modules: aksparpub.o api24.o
	$(LD) -r -x -o aksparlnx.o aksparpub.o api24.o

api24.o: api.o_shipped.x86
	cp $^ $@

api.o: api.o_shipped.x86 api.o_shipped.x86_64
	cp api.o_shipped.$(TARGETARCH) $@

-include $(KERNSRC)/Rules.make

clean:
	rm -rf .tmp_versions
	rm -f .a*.* *.o *.ko aksparlnx.mod.c *.mod *.order *.symvers .*.cmd

# Local Variables: #
# mode: makefile #
# End: #
