HCD_TARGET = usb-vhci-hcd
IOCIFC_TARGET = usb-vhci-iocifc
OBJS = $(HCD_TARGET).o $(IOCIFC_TARGET).o
MDIR = drivers/usb/host

#DESTDIR = 
PREFIX = $(DESTDIR)

BUILD_PREFIX = $(PREFIX)
#INSTALL_PREFIX = $(PREFIX)
INSTALL_PREFIX = $(DESTDIR)

EXTRA_CFLAGS = -DEXPORT_SYMTAB -DKBUILD_EXTMOD -DINCLUDE_CORE_HCD=\"$(PATH_TO_HCDH)\" -DKERNEL_VER=${KVERSION_VERSION}

# Version
VHCI_HCD_VERSION = 1.15.1
USB_VHCI_HCD_VERSION = $(VHCI_HCD_VERSION)
USB_VHCI_IOCIFC_VERSION = $(VHCI_HCD_VERSION)

DIST_DIRS = patch test
DIST_FILES = AUTHORS ChangeLog COPYING INSTALL Makefile NEWS readme.md TODO dkms.conf usb-vhci-hcd.c usb-vhci-iocifc.c usb-vhci-hcd.h usb-vhci.h usb-vhci-dump-urb.c patch/Kconfig.patch test/Makefile test/test.c

# Source dir for dkms (Каталог исходнико в для dkms)
DKMS_DST_DIR := $(DESTDIR)/usr/src/$(HCD_TARGET)-$(VHCI_HCD_VERSION)

# makefile dir (Каталог makefile)
#PWD := $(shell pwd)
mkfile_path := $(abspath $(lastword $(MAKEFILE_LIST)))
PWD := $(patsubst %/,%,$(dir $(mkfile_path)))


# Kernel version (Имя ядра = 5.2.0-0.bpo.2-amd64)
KVERSION := $(shell uname -r)

# kernel subversions (Версия через make функции)
# 5.2.0 0.bpo.2 amd64
KVERSION_WORDS = $(subst -,  ,$(KVERSION))
# 5 2 0
KVERSION_LEVELS = $(subst .,  ,$(firstword $(KVERSION_WORDS)))
# 5
KVERSION_VERSION = $(firstword $(KVERSION_LEVELS))
# 2
KVERSION_PATCHLEVEL = $(word 2, $(KVERSION_LEVELS))
# 0
KVERSION_SUBLEVEL = $(word 3, $(KVERSION_LEVELS))

# 5.2.0-0.bpo.2 
KVERSION_NAME = $(word 1, $(KVERSION_WORDS))-$(word 2, $(KVERSION_WORDS))

# Dir with makefile kernel modules (Каталог с makefile модулей ядра)
KDIR = $(BUILD_PREFIX)/lib/modules/$(KVERSION)/build

INSTALL_DIR = $(INSTALL_PREFIX)/lib/modules/$(KVERSION)
DEST = $(INSTALL_DIR)/kernel/$(MDIR)

# Dir kerenl sources (Каталог исходников ядра для патча)
KSRC = $(KDIR)
#KSRC = $(HEADERS_DIR)

CONF_H = conf/usb-vhci.config.h

# New find hcd.h in kernel headers (Новый способ поиска hcd.h в заголовках ядра)

# must be in /usr/src/linux-headers-$(KVERSION_NAME)-common/include/linux/usb/hcd.h (нужно искать в /usr/src/linux-headers-$(KVERSION_NAME)-common/include/linux/usb/hcd.h)

# Dir list for find hcd.h with space split (Список каталогов для поиска hcd.h разделенный пробелами)
DIRS_TO_HCDH := /usr/src/linux-headers-$(KVERSION_NAME)*/include/linux/usb /usr/src/linux-headers-$(firstword $(KVERSION_WORDS))-*/include/linux/usb $(PWD)/linux/$(KVERSION_VERSION).$(KVERSION_PATCHLEVEL).$(KVERSION_SUBLEVEL)/drivers/usb/core $(KDIR)/drivers/usb/core $(KDIR)/include/linux/usb

# First founded hcd.h in list (Берем первый файл из всех существующих файлов в каталогах)
PATH_TO_HCDH := $(firstword $(foreach dir,$(DIRS_TO_HCDH),$(wildcard $(dir)/hcd.h)))



obj-m := $(OBJS)

# make -C $(KDIR) SUBDIRS=$(PWD) PWD=$(PWD) BUILD_PREFIX=$(BUILD_PREFIX) KDIR=$(KDIR) KVERSION=$(KVERSION) modules

default: $(CONF_H) 
	
	
#	@echo KVERSION=$(KVERSION)
#	@echo KVERSION_NAME=$(KVERSION_NAME)
#	@echo KVERSION_VERSION=$(KVERSION_VERSION)
#	@echo KVERSION_PATCHLEVEL=$(KVERSION_PATCHLEVEL)
	 

	make -C $(KDIR) M=$(PWD) PWD=$(PWD) BUILD_PREFIX=$(BUILD_PREFIX) KDIR=$(KDIR) KVERSION=$(KVERSION) modules
#	make -C $(KDIR) M=$(PWD) PWD=$(PWD) BUILD_PREFIX=$(BUILD_PREFIX) KDIR=$(KDIR) KVERSION=$(KVERSION) modules
.PHONY: default
.SUFFIXES:

ifneq (,$(INSTALL_PREFIX))
install-module:
	echo Dest is $(DEST)
	mkdir -v -p $(DEST) && cp -v $(HCD_TARGET).ko $(IOCIFC_TARGET).ko $(DEST) #&& /sbin/depmod -a -b $(INSTALL_PREFIX) $(KVERSION)
else
install-module:
	echo Dest is $(DEST)
	mkdir -v -p $(DEST) && cp -v $(HCD_TARGET).ko $(IOCIFC_TARGET).ko $(DEST) #&& /sbin/depmod -a $(KVERSION)
endif
.PHONY: install-module

install-devel:
	mkdir -v -p $(INSTALL_PREFIX)/usr/include/linux/ && cp -v -p usb-vhci.h $(INSTALL_PREFIX)/usr/include/linux/
.PHONY: install-devel

install: install-module install-devel
	# echo Dest is $(DEST) 

.PHONY: install

clean-conf:
	-rm -f $(CONF_H)
	-rm -rf conf/
.PHONY: clean-conf

clean: clean-test clean-conf
	-rm -f .*.d .*.o *.o *.ko .*.cmd .*.flags *.mod *.mod.c Module.symvers Module.markers modules.order
	-rm -rf .tmp_versions/
	-rm -rf $(TMP_MKDIST_ROOT)/
.PHONY: clean

patchkernel: $(CONF_H)
	cp -v usb-vhci-hcd.{c,h} usb-vhci-iocifc.c usb-vhci-dump-urb.c $(CONF_H) $(KSRC)/$(MDIR)/
	cp -v usb-vhci.h $(KSRC)/include/linux/
	cd $(KSRC)/$(MDIR); grep -q $(HCD_TARGET).o Makefile || echo "obj-\$$(CONFIG_USB_VHCI_HCD)	+= $(HCD_TARGET).o" >>Makefile
	cd $(KSRC)/$(MDIR); grep -q $(IOCIFC_TARGET).o Makefile || echo "obj-\$$(CONFIG_USB_VHCI_IOCIFC)	+= $(IOCIFC_TARGET).o" >>Makefile
	cd $(KSRC)/$(MDIR)/..; grep -q CONFIG_USB_VHCI_HCD Makefile || echo "obj-\$$(CONFIG_USB_VHCI_HCD)	+= host/" >>Makefile
	cd $(KSRC)/$(MDIR); patch -N -i $(PWD)/patch/Kconfig.patch || :
	if [ "$(KVERSION_VERSION)" -eq 2 -a "$(KVERSION_PATCHLEVEL)" -eq 6 -a "$(KVERSION_SUBLEVEL)" -lt 35 ]; then \
		sed -i -e 's,<linux/usb/hcd.h>,"../core/hcd.h",' $(KSRC)/$(MDIR)/usb-vhci-hcd.h; \
	fi
.PHONY: patchkernel

clean-srcdox:
	-rm -rf html/ vhci-hcd.tag
.PHONY: clean-srcdox

srcdox: clean-srcdox
	mkdir -p html/
	doxygen
	-rm -f vhci-hcd.tag
.PHONY: srcdox

$(CONF_H):
	$(MAKE) testconfig

#TESTMAKE = make -C $(KDIR) SUBDIRS=$(PWD)/test PWD=$(PWD)/test BUILD_PREFIX=$(BUILD_PREFIX) KDIR=$(KDIR) KVERSION=$(KVERSION) EXTRA_CFLAGS='-Wno-unused $(EXTRA_CFLAGS) $1' modules

TESTMAKE = make -C $(KDIR) M=$(PWD)/test PWD=$(PWD)/test BUILD_PREFIX=$(BUILD_PREFIX) KDIR=$(KDIR) KVERSION=$(KVERSION) EXTRA_CFLAGS='-Wno-unused $(EXTRA_CFLAGS) $1' modules


testcc: clean-test
	$(call TESTMAKE)
.PHONY: testcc

clean-test:
	-rm -f test/.*.d test/.*.o test/*.o test/*.ko test/.*.cmd test/.*.flags test/*.mod test/*.mod.c test/Module.symvers test/Module.markers test/modules.order
	-rm -rf test/.tmp_versions/
.PHONY: clean-test

testconfig: testcc
	mkdir -p conf/
	echo "// do not edit; automatically generated by 'make testconfig' in vhci-hcd sourcedir" >$(CONF_H)
	echo "#define USB_VHCI_HCD_VERSION \"$(USB_VHCI_HCD_VERSION)\"" >>$(CONF_H)
	echo "#define USB_VHCI_HCD_DATE \"$(shell date +"%F")\"" >>$(CONF_H)
	echo "#define USB_VHCI_IOCIFC_VERSION \"$(USB_VHCI_IOCIFC_VERSION)\"" >>$(CONF_H)
	echo "#define USB_VHCI_IOCIFC_DATE USB_VHCI_HCD_DATE" >>$(CONF_H)
	$(MAKE) clean-test
	if $(call TESTMAKE,-DTEST_GIVEBACK_MECH) >/dev/null 2>&1; then \
		echo "//#define OLD_GIVEBACK_MECH" >>$(CONF_H); \
	else \
		echo "#define OLD_GIVEBACK_MECH" >>$(CONF_H); \
	fi
	$(MAKE) clean-test
	if $(call TESTMAKE,-DTEST_DEV_BUS_ID) >/dev/null 2>&1; then \
		echo "//#define OLD_DEV_BUS_ID" >>$(CONF_H); \
	else \
		echo "#define OLD_DEV_BUS_ID" >>$(CONF_H); \
	fi
	$(MAKE) clean-test
	if $(call TESTMAKE,-DTEST_DEV_INIT_NAME) >/dev/null 2>&1; then \
		echo "//#define NO_DEV_INIT_NAME" >>$(CONF_H); \
	else \
		echo "#define NO_DEV_INIT_NAME" >>$(CONF_H); \
	fi
	$(MAKE) clean-test
	if $(call TESTMAKE,-DTEST_HAS_TT_FLAG) >/dev/null 2>&1; then \
		echo "//#define NO_HAS_TT_FLAG" >>$(CONF_H); \
	else \
		echo "#define NO_HAS_TT_FLAG" >>$(CONF_H); \
	fi
	echo "// end of file" >>$(CONF_H)
.PHONY: testconfig

config:
	@echo "**********************************************************"; \
	echo " Please answer the following questions." ; \
	echo " Your answers will influence the creation of $(CONF_H)"; \
	echo " which is needed to build this vhci-hcd driver."; \
	echo "**********************************************************"; \
	echo; \
	echo "NOTE: You can let me do this for you automatically without answering this"; \
	echo "      questions by running 'make testconfig'. I will compile a few test modules"; \
	echo "      for the target kernel which helps me guessing the answers. So if this"; \
	echo "      is not possible on the currently running system and you just want to"; \
	echo "      patch the sources into a kernel source tree, then answering them by"; \
	echo "      yourself is the right thing to do."; \
	echo; \
	echo "NOTE: You can cancel this at any time (by pressing CTRL-C). $(CONF_H)"; \
	echo "      will not be overwritten then."; \
	echo; \
	echo "Question 1 of 4:"; \
	echo "  What does the signature of usb_hcd_giveback_urb look like?"; \
	echo "   a) usb_hcd_giveback_urb(struct usb_hcd *, struct urb *, int)    <-- recent kernels"; \
	echo "   b) usb_hcd_giveback_urb(struct usb_hcd *, struct urb *)         <-- older kernels"; \
	echo "  You may find it in <KERNEL_SRCDIR>/drivers/usb/core/hcd.h."; \
	OLD_GIVEBACK_MECH=; \
	while true; do \
		echo -n "Answer (a/b): "; \
		read ANSWER; \
		if [ "$$ANSWER" = a ]; then break; \
		elif [ "$$ANSWER" = b ]; then \
			OLD_GIVEBACK_MECH=y; \
			break; \
		fi; \
	done; \
	echo; \
	echo "Question 2 of 4:"; \
	echo "  Are the functions dev_name and dev_set_name defined?"; \
	echo "  You may find them in <KERNEL_SRCDIR>/include/linux/device.h."; \
	OLD_DEV_BUS_ID=; \
	while true; do \
		echo -n "Answer (y/n): "; \
		read ANSWER; \
		if [ "$$ANSWER" = y ]; then break; \
		elif [ "$$ANSWER" = n ]; then \
			OLD_DEV_BUS_ID=y; \
			break; \
		fi; \
	done; \
	echo; \
	echo "Question 3 of 4:"; \
	echo "  Does the device structure has the init_name field?"; \
	echo "  You may check <KERNEL_SRCDIR>/include/linux/device.h to find out."; \
	echo "  It is always safe to answer 'n'."; \
	NO_DEV_INIT_NAME=; \
	while true; do \
		echo -n "Answer (y/n): "; \
		read ANSWER; \
		if [ "$$ANSWER" = y ]; then break; \
		elif [ "$$ANSWER" = n ]; then \
			NO_DEV_INIT_NAME=y; \
			break; \
		fi; \
	done; \
	echo; \
	echo "Question 4 of 4:"; \
	echo "  Does the usb_hcd structure has the has_tt field?"; \
	echo "  This field was added in kernel version 2.6.35."; \
	NO_HAS_TT_FLAG=; \
	while true; do \
		echo -n "Answer (y/n): "; \
		read ANSWER; \
		if [ "$$ANSWER" = y ]; then break; \
		elif [ "$$ANSWER" = n ]; then \
			NO_HAS_TT_FLAG=y; \
			break; \
		fi; \
	done; \
	echo; \
	echo "Thank you"; \
	mkdir -p conf/; \
	echo "// do not edit; automatically generated by 'make config' in vhci-hcd sourcedir" >$(CONF_H); \
	echo "#define USB_VHCI_HCD_VERSION \"$(USB_VHCI_HCD_VERSION)\"" >>$(CONF_H); \
	echo "#define USB_VHCI_HCD_DATE \"$(shell date +"%F")\"" >>$(CONF_H); \
	echo "#define USB_VHCI_IOCIFC_VERSION \"$(USB_VHCI_IOCIFC_VERSION)\"" >>$(CONF_H); \
	echo "#define USB_VHCI_IOCIFC_DATE USB_VHCI_HCD_DATE" >>$(CONF_H); \
	if [ -z "$$OLD_GIVEBACK_MECH" ]; then \
		echo "//#define OLD_GIVEBACK_MECH" >>$(CONF_H); \
	else \
		echo "#define OLD_GIVEBACK_MECH" >>$(CONF_H); \
	fi; \
	if [ -z "$$OLD_DEV_BUS_ID" ]; then \
		echo "//#define OLD_DEV_BUS_ID" >>$(CONF_H); \
	else \
		echo "#define OLD_DEV_BUS_ID" >>$(CONF_H); \
	fi; \
	if [ -z "$$NO_DEV_INIT_NAME" ]; then \
		echo "//#define NO_DEV_INIT_NAME" >>$(CONF_H); \
	else \
		echo "#define NO_DEV_INIT_NAME" >>$(CONF_H); \
	fi; \
	if [ -z "$$NO_HAS_TT_FLAG" ]; then \
		echo "//#define NO_HAS_TT_FLAG" >>$(CONF_H); \
	else \
		echo "#define NO_HAS_TT_FLAG" >>$(CONF_H); \
	fi; \
	echo "// end of file" >>$(CONF_H)
.PHONY: config

TMP_MKDIST_ROOT = .tmp_make_dist
TMP_MKDIST = $(TMP_MKDIST_ROOT)/vhci-hcd-$(VHCI_HCD_VERSION)

dist:
	-rm -rf $(TMP_MKDIST_ROOT)/
	mkdir -p $(TMP_MKDIST)
	$(foreach x,$(DIST_DIRS),mkdir -p $(TMP_MKDIST)/$(x);)
	$(foreach x,$(DIST_FILES),cp -p $(x) $(TMP_MKDIST)/$(x);)
	cp -p -R linux/ $(TMP_MKDIST)/
	(cd $(TMP_MKDIST_ROOT)/; tar -c vhci-hcd-$(VHCI_HCD_VERSION)) | bzip2 -cz9 >vhci-hcd-$(VHCI_HCD_VERSION).tar.bz2
	(cd $(TMP_MKDIST_ROOT)/; tar -c vhci-hcd-$(VHCI_HCD_VERSION)) | gzip -c >vhci-hcd-$(VHCI_HCD_VERSION).tar.gz
	-rm -rf $(TMP_MKDIST_ROOT)/
.PHONY: dist

# Copy sources to dkms dir (Копирует исходники в каталог dkms)
dkmscopy: dkmsclean
	# Создать каталог
	mkdir -p $(DKMS_DST_DIR)
	# Создать подкаталоги исходников
	$(foreach dir ,$(DIST_DIRS), mkdir -p $(DKMS_DST_DIR)/$(dir);)
	# Скопировать исходники
	$(foreach filename,$(DIST_FILES), cp -T $(PWD)/$(filename) $(DKMS_DST_DIR)/$(filename);)

.PHONY: dkmscopy

# Remove sources from dkms dir (Удаляет исходники из каталога dkms)
dkmsclean:
	# Удалить каталог
	rm -f -r $(DKMS_DST_DIR)

.PHONY: dkmsclean


# Add module to dkms (Добавляет модуль в dkms)
dkmsadd: 
	# Добавление модуля в dkms
	dkms add -m $(HCD_TARGET) -v $(VHCI_HCD_VERSION) 
.PHONY: dkmsadd

# Build dkms module (Собирает модуль в dkms)
dkmsbuild: 
	dkms build -m $(HCD_TARGET) -v $(VHCI_HCD_VERSION) -k $(KVERSION)
.PHONY: dkmsbuild

# Install dkms (Устанавливает модуль в dkms)
dkmsinstall: 
	dkms install -m $(HCD_TARGET) -v $(VHCI_HCD_VERSION) -k $(KVERSION)
.PHONY: dkmsinstall

# Uninstall dkms (Отменяет установку модуля в dkms)
dkmsuninstall: 
	dkms uninstall -m $(HCD_TARGET) -v $(VHCI_HCD_VERSION) -k $(KVERSION)
.PHONY: dkmsuninstall

# Remove dkms (Удаляет модуль из dkms)
dkmsremove: 
	dkms remove -m $(HCD_TARGET) -v $(VHCI_HCD_VERSION) --all
.PHONY: dkmsremove

# Make deb for dkms (Создает deb в dkms)
deb: 
	dkms mkdeb -m $(HCD_TARGET) -v $(VHCI_HCD_VERSION) -k $(KVERSION) --source-only
.PHONY: mkdeb

# Make binary deb (Создает бинарный deb)
bindeb: 
	dkms mkbmdeb -m $(HCD_TARGET) -v $(VHCI_HCD_VERSION) -k $(KVERSION)
.PHONY: mkbmdeb

-include $(KDIR)/Rules.make
