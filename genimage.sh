#!/bin/bash

SKETCHPATH=$1
if [ "$1" == "" ]; then
	echo "Usage: $0 <sketch-file>"
	exit 1
fi

WHOAMI=`whoami`
SUDO=`which sudo`
MOUNTCMD=`which mount`
UMOUNTCMD=`which umount`
CP=""
PASS=0
NEEDUMOUNT=`mount | grep arduinoimg -c`


if [ "$WHOAMI" == "root" ]; then
	PASS=1
elif [ "$SUDO" != "" ]; then
	MOUNTCMD="$SUDO $MOUNTCMD"
	UMOUNTCMD="$SUDO $UMOUNTCMD"
	CP="$SUDO cp"
	PASS=1
fi

if [ $PASS -eq 0 ]; then
	echo "Either root permissions or sudo is required"
	exit 1
fi

echo "Cleaning up environment..."
mkdir -p /tmp/arduinoimg
rm -rf build/image-full-galileo/

echo "Copying necessary files..."
cp -r image-full-galileo build/

echo "Modifying image filesystem..."
$MOUNTCMD -t ext2 -o loop build/image-full-galileo/image-full-galileo-clanton.ext3 /tmp/arduinoimg
$CP $SKETCHPATH /tmp/arduinoimg/sketch/sketch.elf
$UMOUNTCMD -f /tmp/arduinoimg

echo "Final cleanup..."
rmdir /tmp/arduinoimg

echo "**********"
echo "Image created at build/image-full-galileo/"
echo "**********"
