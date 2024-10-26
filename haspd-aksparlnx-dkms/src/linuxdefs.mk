# $Id: linuxdefs.mk,v 1.2 2007/03/17 17:08:57 lav Exp $
#
# global make defines for Unix-like systems
# (c) 2000-2004 by Aladdin Knowledge Systems, Germany
#

GLOBDEF_UNIX = -DWE_ARE_UNIX

# Linux compilers

LINUX_PREFIX   =
LINUX_CC       = $(LINUX_PREFIX)gcc
LINUX_LD       = $(LINUX_PREFIX)ld
LINUX_AR       = $(LINUX_PREFIX)ar
LINUX_STRIP    = $(LINUX_PREFIX)strip
LINUX_CPLUS    = $(LINUX_PREFIX)g++
LINUX_COPTS    = -O2 -fomit-frame-pointer $(GLOBDEF_UNIX) #-fno-strength-reduce
LINUX_DEBOPTS  = -g $(GLOBDEF_UNIX)
LINUX_DEFINE   = -DLINUX
LINUX_PIC      = -fpic
LINUX_ANSI     = -ansi
LINUX_PACK     = -fpack-struct
LINUX_WARNFULL = -W -Wall -Wstrict-prototypes
LINUX_WARNNONE = -w
LINUX_WARNERR  = -Werror
LINUX_I386_OPTS       = -mcpu=i386 -march=i386 -D__32BIT__ -D_AKS_LITTLE_ENDIAN_
LINUX_I386_KERN_OPTS  =
LINUX_M68K_OPTS       = -m68040 -D__32BIT__ -D_AKS_BIG_ENDIAN_
LINUX_M68K_KERN_OPTS  =
LINUX_MIPS_OPTS       = -mtune=r3000 -mips1 -D__32BIT__ -D_AKS_LITTLE_ENDIAN_ -D_AKS_NO_UNALIGN_
LINUX_MIPS_KERN_OPTS  =
LINUX_AMD64_OPTS      = -D__64BIT__ -D_AKS_LITTLE_ENDIAN_
LINUX_AMD64_KERN_OPTS =
LINUX_IA64_OPTS       = -D__64BIT__ -D_AKS_LITTLE_ENDIAN_ -D_AKS_NO_UNALIGN_
LINUX_IA64_KERN_OPTS  = -Wa,-x -ffixed-r13 -mfixed-range=f10-f15,f32-f127 -funwind-tables
LINUX_PPC_OPTS        = -D__32BIT__ -D_AKS_BIG_ENDIAN_
LINUX_PPC_KERN_OPTS   = -fsigned-char -msoft-float -ffixed-r2 -mmultiple -mstring
LINUX_ALPHA_OPTS      = -mcpu=21064 -mieee -D__64BIT__ -D_AKS_LITTLE_ENDIAN_ -D_AKS_NO_UNALIGN_
LINUX_ALPHA_KERN_OPTS = -ffixed-8 -mno-fp-regs
LINUX_ARMEL_OPTS      = -D__32BIT__ -D_AKS_LITTLE_ENDIAN_ -D_AKS_NO_UNALIGN_
LINUX_ARMEL_KERN_OPTS =
LINUX_CPLUS_COPTS     = $(LINUX_COPTS) -frtti -fexceptions
LINUX_CPLUS_DEBOPTS   = $(LINUX_DEBOPTS) -frtti -fexceptions

