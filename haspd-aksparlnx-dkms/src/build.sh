#!/bin/sh
# $Id: build.sh,v 1.4 2008/10/20 08:02:04 lav Exp $
#
# * distinguish between kernel versions (2.4.x and 2.6.x) and invoke
#   make accordingly to build aksparlnx
# * provide a rough way to install the built aksparlnx
#
# Copyright (c) 2005 by Aladdin Knowledge Systems Ltd.

MYNAME=$0
unset VERBOSE INSTALL INSTALLONLY
MODULENAME=aksparlnx
MODULEVERSION=1.7
KERNELVERSION=`uname -r`
MAJVERSION=$(echo $KERNELVERSION | cut -c1-3)

if [ $# -gt 0 ]; then
	while [ ! -z "$1" ]; do
		case $1 in
			-h | --help)
				echo "usage: $MYNAME [--install|--remove] [--verbose]"
				exit 0;
				;;
			-v | --verbose)
				VERBOSE=--verbose;
				;;
			-i | --install)
				INSTALL=1;
				;;
			-o | --installonly)
				INSTALLONLY=1;
				;;
			-r | --remove)
				REMOVE=1;
				;;
			*)
				echo "usage: $MYNAME [--install|--remove] [--verbose]" 1>&2
				exit 13;
				;;
		esac
		shift
	done
fi

dkms_build_module()
{
    STATUS=`dkms status -m $MODULENAME -v $MODULEVERSION`

    if [ ! -z "$REMOVE" ] ; then
        if [ "$STATUS" ] ; then
	    dkms remove -m $MODULENAME -v $MODULEVERSION --all
	    rm -r /usr/src/$MODULENAME-$MODULEVERSION
	else
	    echo "dkms module $MODULENAME-$MODULEVERSION is not installed"
	fi
        exit 0;
    fi

    [ "$STATUS" ] || dkms add -m $MODULENAME -v $MODULEVERSION
    [ "$STATUS" ] && dkms uninstall -m $MODULENAME -v $MODULEVERSION -k $KERNELVERSION
    dkms build -m $MODULENAME -v $MODULEVERSION -k $KERNELVERSION
    dkms install -m $MODULENAME -v $MODULEVERSION -k $KERNELVERSION
    exit 0;
}

# Если заметили наличие dkms и пакета, запускаем сборку
if [ -r ./dkms.conf ] && [ `which dkms 2>/dev/null` ] ; then
    mkdir -p /usr/src/$MODULENAME-$MODULEVERSION
    cp * /usr/src/$MODULENAME-$MODULEVERSION
    dkms_build_module
fi

# source and destination directories can be inherited from the environment

if [ -z "$KERNSRC" ]; then
	KERNSRC=/lib/modules/$KERNELVERSION/build
fi
if [ -z "$MODDEST" ]; then
	MODDEST=/lib/modules/$KERNELVERSION/misc
fi

EXT=.ko
[ "$MAJVERSION" = "2.4" ] && EXT=.o

# process remove action

if [ ! -z "$REMOVE" ]; then

	# remove cannot be combined with install or installonly

	if [ ! -z "$INSTALL" -o ! -z "$INSTALLONLY" ]; then
		echo "remove action cannot be combined with install action"  1>&2
		echo "aborting" 1>&2
		exit 13
	fi

	# if it's not installed we don't need to remove it

	if [ ! -f $MODDEST/aksparlnx$EXT ]; then
		if [ ! -z "$VERBOSE" ]; then
			echo
			echo "aksparlnx$EXT is not installed"
			echo
		fi
		exit 0
	fi

	# removing needs root rights

#	if [ $EUID -ne 0 ]; then
	if [ $(id -u) -ne 0 ]; then

		echo
		echo "Becoming root@$(uname -n) to remove aksparlnx$EXT."
		# rerun myself with root rights
		su --command="$MYNAME $VERBOSE --remove" root
		ST=$?
		if [ $ST -ne 0 ]; then
			echo "su failed" 1>&2
			echo "aborting" 1>&2
			exit 14
		fi

	else

		rm -f $MODDEST/aksparlnx$EXT
		PATH=/sbin:/bin:$PATH depmod -a
		ST=$?
		if [ $ST -ne 0 ]; then
			echo "cannot update module dependencies" 1>&2
			echo "aborting" 1>&2
			exit 15
		fi

		if [ ! -z "$VERBOSE" ]; then
			echo
			echo "aksparlnx$EXT removed from $MODDEST"
			echo
		fi
	fi

	exit 0
fi   # ! -z "$REMOVE"


# build module except if installonly is requested

if [ -z "$INSTALLONLY" ]; then

	# check availability of kernel source

	if [ ! -f $KERNSRC/include/linux/version.h ]; then
		if [ ! -f $KERNSRC/include/generated/uapi/linux/version.h ]; then
			echo "no kernel sources found at $KERNSRC or kernel not configured" 1>&2
			echo "use KERNSRC to set correct location" 1>&2
			echo "aborting" 1>&2
			exit 11
		fi
	fi

	# build aksparlnx.o/.ko

	if [ $MAJVERSION = "2.4" ]; then
		# if there is a .config file in the kernel source directory use the kernel makefile like with 2.6
		if [ -f $KERNSRC/.config ]; then
			if [ ! -z "$VERBOSE" ]; then
				echo "using $KERNSRC/Makefile"
			fi
			make kernel24 KERNSRC=$KERNSRC
			ST=$?
		else
			if [ ! -z "$VERBOSE" ]; then
				echo "using own Makefile"
			fi
			make bymyself KERNSRC=$KERNSRC
			ST=$?
		fi
	elif [ $MAJVERSION = "2.6" ]; then
		make kernel26 KERNSRC=$KERNSRC
		ST=$?
	else
		make kernel3 KERNSRC=$KERNSRC
		ST=$?
	fi

	if [ $ST -eq 0 ]; then
		if [ ! -f aksparlnx$EXT ]; then
			echo "aksparlnx$EXT does not exist!"  1>&2
			echo "aborting" 1>&2
			exit 12
		else
			if [ ! -z "$VERBOSE" ]; then
				echo
				echo "aksparlnx$EXT successfully built:"
				ls -l aksparlnx$EXT
				echo
			fi
		fi
	fi
fi   # -z "$INSTALLONLY"


# if install or installonly is requested, install the module

if [ ! -z "$INSTALLONLY" -o ! -z "$INSTALL" ]; then

	if [ ! -f aksparlnx$EXT ]; then
		echo "aksparlnx$EXT does not exist!" 1>&2
		echo "aborting" 1>&2
		exit 12
	fi

		mkdir -p $MODDEST
		install -m 644 -o root -g root aksparlnx$EXT $MODDEST
		ST=$?
		if [ $ST -ne 0 ]; then
			echo "cannot install aksparlnx$EXT into $MODDEST" 1>&2
			echo "aborting" 1>&2
			exit 14
		fi

		PATH=/sbin:/bin:$PATH depmod -a
		ST=$?
		if [ $ST -ne 0 ]; then
			echo "cannot update module dependencies" 1>&2
			echo "aborting" 1>&2
			exit 15
		fi

		if [ ! -z "$VERBOSE" ]; then
			echo
			echo "aksparlnx$EXT installed into $MODDEST"
			echo
		fi

fi   # ! -z "$INSTALLONLY" -o ! -z "$INSTALL"

exit $ST
