#!/bin/bash

SKETCHPATH=$1
if [ "$1" == "" ]; then
	echo "Usage: $0 <sketch-file>"
	exit 1
fi

MOUNTDIR=/tmp/arduinoimg
WHOAMI=`whoami`
SUDO=`which sudo`
FUSEEXT=`which fuseext2`
MOUNTCMD=`which mount`
UMOUNTCMD=`which umount`
CP="cp"
PASS=0
NEEDUMOUNT=`mount | grep arduinoimg -c`
MOUNTOPTS="-t ext2 -o loop"
UNAME_S=`uname -s`

if [ "$UNAME_S" == "Darwin" ]; then
	MOUNTDIR=/var/tmp/arduinoimg
	SUDO=""
	WHOAMI=""
	FUSEEXT=`which fuse-ext2`
fi

if [ "$WHOAMI" == "root" ]; then
	PASS=1
	UMOUNTCMD="$UMOUNTCMD -f"
elif [ "$FUSEEXT" != "" ]; then
	MOUNTCMD=$FUSEEXT
	if [ "$UNAME_S" != "Darwin" ]; then
		UMOUNTCMD=`which fusermount`
		UMOUNTCMD="${UMOUNTCMD} -u"
	fi
	MOUNTOPTS="-o rw+"
	PASS=1
elif [ "$SUDO" != "" ]; then
	MOUNTCMD="$SUDO $MOUNTCMD"
	UMOUNTCMD="$SUDO $UMOUNTCMD -f"
	CP="$SUDO cp"
	PASS=1
fi

if [ $PASS -eq 0 ]; then
	if [ "$UNAME_S" == "Darwin" ]; then
		echo "fuse-ext2 is required";
	else
		echo "Root permissions, sudo or fuse-ext2 is required"
	fi
	exit 1
fi

echo "Cleaning up environment..."
mkdir -p $MOUNTDIR
rm -rf build/image-full-galileo/

echo "Copying necessary files..."
tar xvf SDCard.1.0.4.tar.bz2
cp -r image-full-galileo build/
rm -rf image-full-galileo

echo "Modifying image filesystem..."
$MOUNTCMD build/image-full-galileo/image-full-galileo-clanton.ext3 $MOUNTDIR ${MOUNTOPTS}

if [ "$UNAME_S" == "Darwin" ]; then
	sleep 2
fi

$CP $SKETCHPATH $MOUNTDIR/sketch/sketch.elf
$UMOUNTCMD $MOUNTDIR

echo "Final cleanup..."
rmdir $MOUNTDIR

echo "**********"
echo "Image created at build/image-full-galileo/"
echo "**********"
