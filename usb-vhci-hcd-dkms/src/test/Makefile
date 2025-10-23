TARGET = test
OBJS = test.o
ccflags-y = $(EXTRA_CFLAGS)

obj-m      := $(TARGET).o

default:
	make -C $(KDIR) SUBDIRS=$(PWD) PWD=$(PWD) BUILD_PREFIX=$(BUILD_PREFIX) KDIR=$(KDIR) KVERSION=$(KVERSION) EXTRA_CFLAGS='$(EXTRA_CFLAGS)' modules
.PHONY: default
.SUFFIXES:

-include $(KDIR)/Rules.make
