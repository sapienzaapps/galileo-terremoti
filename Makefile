
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
	echo "#define GEN1 1" >> buildcfg.h
	echo "#define GEN2 0" >> buildcfg.h
	echo "$$BUILDCFG_FOOTER" >> buildcfg.h

gen2prep:
	echo "$$BUILDCFG_HEADER" > buildcfg.h
	echo "#define GEN1 0" >> buildcfg.h
	echo "#define GEN2 1" >> buildcfg.h
	echo "$$BUILDCFG_FOOTER" >> buildcfg.h

gen1: gen1prep
	arduino --verify --verbose --board intel:i586-uclibc:izmir_fd --pref build.path=build --pref update.check=false galileo-terremoti.ino

gen2: gen2prep
	arduino --verify --verbose --board intel:i586-uclibc:izmir_fg --pref build.path=build --pref update.check=false galileo-terremoti.ino

gen1upload: gen1
	#arduino --upload --verbose --board intel:i586-uclibc:izmir_fd --pref build.path=build --pref update.check=false galileo-terremoti.ino
	/bin/bash --verbose --noprofile ${ARDUINOROOT}/hardware/intel/i586-uclibc/tools/izmir/clupload_linux.sh ${ARDUINOROOT}/hardware/tools ${CWD}/build/galileo-terremoti.cpp.elf ${SERIAL}

gen2upload: gen2
	#arduino --upload --verbose --board intel:i586-uclibc:izmir_fg --pref build.path=build --pref update.check=false galileo-terremoti.ino
	/bin/bash --verbose --noprofile ${ARDUINOROOT}/hardware/intel/i586-uclibc/tools/izmir/clupload_linux.sh ${ARDUINOROOT}/hardware/tools ${CWD}/build/galileo-terremoti.cpp.elf ${SERIAL}

clean:
	rm -rf build/
