
define BUILDCFG_HEADER

#ifndef __BUILDCFG_H
#define __BUILDCFG_H
#define __IS_GALILEO

endef

define BUILDCFG_FOOTER
#endif
endef

export BUILDCFG_HEADER
export BUILDCFG_FOOTER
ARDUINOFILE=$(shell which arduino)
ARDUINOROOT=$(shell dirname ${ARDUINOFILE})
CWD=$(shell pwd)
WHOAMI=$(shell whoami)
SUDOEXISTS=$(shell which sudo)
MOUNTCMD=""
UMOUNTCMD=""
CP=""
PASS=0
NEEDUMOUNT=$(shell mount | grep arduinoimg -c)

ifneq (${SUDOEXISTS}, )
MOUNTCMD=$(shell which sudo) $(shell which mount)
UMOUNTCMD=$(shell which sudo) $(shell which umount)
CP=$(shell which sudo) cp
PASS=1
endif
ifeq (${WHOAMI}, "root")
MOUNTCMD=$(shell which mount)
UMOUNTCMD=$(shell which umount)
CP=cp
PASS=1
endif


SERIAL=/dev/ttyACM0

ifdef ARDUINODEV
	SERIAL=${ARDUINODEV}
endif

all:
	$(info *********************************************************************)
	$(info No default task available - please see README.md)
	$(info *********************************************************************)
	$(info *********************************************************************)

gen1prep:
	echo "$$BUILDCFG_HEADER" > buildcfg.h
	echo "#define GALILEO_GEN 1" >> buildcfg.h
	echo "$$BUILDCFG_FOOTER" >> buildcfg.h

gen2prep:
	echo "$$BUILDCFG_HEADER" > buildcfg.h
	echo "#define GALILEO_GEN 2" >> buildcfg.h
	echo "$$BUILDCFG_FOOTER" >> buildcfg.h

gen1: gen1prep
	rm -rf build/
	arduino --verify --verbose --board intel:i586-uclibc:izmir_fd --pref build.path=build --pref update.check=false galileo-terremoti.ino

gen2: gen2prep
	rm -rf build/
	arduino --verify --verbose --board intel:i586-uclibc:izmir_fg --pref build.path=build --pref update.check=false galileo-terremoti.ino

gen1upload: gen1
	#arduino --upload --verbose --board intel:i586-uclibc:izmir_fd --pref build.path=build --pref update.check=false galileo-terremoti.ino
	/bin/bash --verbose --noprofile ${ARDUINOROOT}/hardware/intel/i586-uclibc/tools/izmir/clupload_linux.sh ${ARDUINOROOT}/hardware/tools ${CWD}/build/galileo-terremoti.cpp.elf ${SERIAL}

gen2upload: gen2
	#arduino --upload --verbose --board intel:i586-uclibc:izmir_fg --pref build.path=build --pref update.check=false galileo-terremoti.ino
	/bin/bash --verbose --noprofile ${ARDUINOROOT}/hardware/intel/i586-uclibc/tools/izmir/clupload_linux.sh ${ARDUINOROOT}/hardware/tools ${CWD}/build/galileo-terremoti.cpp.elf ${SERIAL}

imageprepare:
ifeq (${PASS}, 0)
	$(error Either root permissions or sudo is required)
endif

gen1image: imageprepare gen1
ifeq (${NEEDUMOUNT}, 1)
	${UMOUNTCMD} -f /tmp/arduinoimg
endif
	mkdir -p /tmp/arduinoimg
	cp -r image-full-galileo build/
	${MOUNTCMD} -t ext2 -o loop build/image-full-galileo/image-full-galileo-clanton.ext3 /tmp/arduinoimg
	${CP} build/galileo-terremoti.cpp.elf /tmp/arduinoimg/sketch/sketch.elf
	${UMOUNTCMD} -f /tmp/arduinoimg
	rmdir /tmp/arduinoimg
	
gen2image: imageprepare gen2
ifeq (${NEEDUMOUNT}, 1)
	${UMOUNTCMD} -f /tmp/arduinoimg
endif
	mkdir -p /tmp/arduinoimg
	cp -r image-full-galileo build/
	${MOUNTCMD} -t ext2 -o loop build/image-full-galileo/image-full-galileo-clanton.ext3 /tmp/arduinoimg
	${CP} build/galileo-terremoti.cpp.elf /tmp/arduinoimg/sketch/sketch.elf
	${UMOUNTCMD} -f /tmp/arduinoimg
	rmdir /tmp/arduinoimg

clean:
ifeq (${NEEDUMOUNT}, 1)
	${UMOUNTCMD} -f /tmp/arduinoimg
endif
	rm -rf build/
