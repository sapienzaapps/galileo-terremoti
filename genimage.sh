#!/bin/bash

PROJECTDIR=$1

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
	echo Either root permissions or sudo is required
	exit 1
fi

mkdir -p /tmp/arduinoimg
cp -r image-full-galileo build/
$MOUNTCMD -t ext2 -o loop build/image-full-galileo/image-full-galileo-clanton.ext3 /tmp/arduinoimg
$CP build/galileo-terremoti.cpp.elf /tmp/arduinoimg/sketch/sketch.elf
$UMOUNTCMD -f /tmp/arduinoimg
rmdir /tmp/arduinoimg