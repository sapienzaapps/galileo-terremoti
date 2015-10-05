#!/bin/bash

SKETCHPATH=$1
if [ "$1" == "" ]; then
	echo "Usage: $0 <sketch-file>"
	exit 1
fi

WHOAMI=`whoami`
SUDO=`which sudo`
FUSEEXT=`which fuseext2`
MOUNTCMD=`which mount`
UMOUNTCMD=`which umount`
CP=""
PASS=0
NEEDUMOUNT=`mount | grep arduinoimg -c`
MOUNTOPTS="-t ext2 -o loop"

if [ "$WHOAMI" == "root" ]; then
	PASS=1
	UMOUNTCMD="$UMOUNTCMD -f"
elif [ "$FUSEEXT" != "" ]; then
	MOUNTCMD=$FUSEEXT
	UMOUNTCMD=`which fusermount`
	UMOUNTCMD="${UMOUNTCMD} -u"
	MOUNTOPTS="-o rw"
	PASS=1
elif [ "$SUDO" != "" ]; then
	MOUNTCMD="$SUDO $MOUNTCMD"
	UMOUNTCMD="$SUDO $UMOUNTCMD -f"
	CP="$SUDO cp"
	PASS=1
fi

if [ $PASS -eq 0 ]; then
	echo "Root permissions, sudo or fuse-ext2 is required"
	exit 1
fi

echo "Cleaning up environment..."
mkdir -p /tmp/arduinoimg
rm -rf build/image-full-galileo/

echo "Copying necessary files..."
cp -r image-full-galileo build/

echo "Modifying image filesystem..."
$MOUNTCMD ${MOUNTOPTS} build/image-full-galileo/image-full-galileo-clanton.ext3 /tmp/arduinoimg
$CP $SKETCHPATH /tmp/arduinoimg/sketch/sketch.elf
$UMOUNTCMD /tmp/arduinoimg

echo "Final cleanup..."
rmdir /tmp/arduinoimg

echo "**********"
echo "Image created at build/image-full-galileo/"
echo "**********"
