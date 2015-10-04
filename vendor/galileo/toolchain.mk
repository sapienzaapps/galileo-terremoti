
# Example found https://github.com/tokoro10g/galileo-makefile/blob/master/Makefile

# Variants:
# galileo_fab_d - Galileo
# galileo_fab_g - Galileo Gen 2

VARIANT        ?= galileo_fab_d

GALILEO_GEN    := 1
ifeq (${VARIANT}, galileo_fab_g)
GALILEO_GEN    := 2
endif

DISTRO         := clanton-tiny
TARGET_NAME    := i586-poky-linux-uclibc
NATIVE_NAME    := x86_64-pokysdk-linux
GCC_VERSION    := 4.7.2

SDK_ROOT       ?= /opt/arduino-1.6.0+Intel/

ifeq (, $(wildcard $(SDK_ROOT)))
ifneq (, $(wildcard /Applications/Arduino.app))
SDK_ROOT        = /Applications/Arduino.app/Contents/Resources/Java/
else
$(error No Arduino SDK found)
endif
endif

SYSROOT_TARGET := $(SDK_ROOT)/hardware/tools/i586/sysroots/$(TARGET_NAME)/
SYSROOT_NATIVE := $(SDK_ROOT)/hardware/tools/i586/sysroots/$(NATIVE_NAME)/

ifeq (, $(wildcard $(SYSROOT_TARGET)))
SYSROOT_TARGET := $(SDK_ROOT)/hardware/tools/i586/$(TARGET_NAME)/
SYSROOT_NATIVE := $(SDK_ROOT)/hardware/tools/i586/pokysdk/
endif

TARGET_ARCH  := -m32 -march=i586 --sysroot=$(SYSROOT_TARGET)
INCLUDE_DIRS := -I $(SYSROOT_TARGET)/usr/include \
				-I $(SYSROOT_TARGET)/usr/include/c++/ \
				-I $(SYSROOT_TARGET)/usr/include/c++/$(TARGET_NAME) \
				-I $(SDK_ROOT)/hardware/intel/i586-uclibc/cores/arduino/ \
				-I $(SDK_ROOT)/hardware/intel/i586-uclibc/variants/$(VARIANT)/ \
				-I $(SYSROOT_NATIVE)/usr/lib/$(TARGET_NAME)/gcc/$(TARGET_NAME)/$(GCC_VERSION)/include \
				-I vendor/${PLATFORM}
LIBRARY_DIRS := -L $(SYSROOT_TARGET)/lib/ \
				-L $(SYSROOT_TARGET)/usr/lib/ \
				-L $(SYSROOT_TARGET)/usr/lib/$(TARGET_NAME)/$(GCC_VERSION)
COMPILE_OPTS := -Os -pipe -g -feliminate-unused-debug-types -fpermissive -Wall -w -fexceptions \
	-ffunction-sections -fdata-sections -MMD -D__ARDUINO_X86__ -Xassembler -mquark-strip-lock=yes -DARDUINO=10600 \
	$(INCLUDE_DIRS)

LFLAGS      := -Wl,-O1 -Wl,--hash-style=gnu -Wl,--as-needed $(LIBRARY_DIRS) -lstdc++ -lpthread $(TARGET_ARCH)
UNAME_S     := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
LFLAGS      := -Os -Wl,--gc-sections -lpthread $(TARGET_ARCH)
endif

TOOLDIR     := $(SYSROOT_NATIVE)/usr/bin/$(TARGET_NAME)
CC          := $(TOOLDIR)/i586-poky-linux-uclibc-gcc
CPP         := $(TOOLDIR)/i586-poky-linux-uclibc-g++
STRIP       := $(TOOLDIR)/i586-poky-linux-uclibc-strip
CFLAGS      := $(COMPILE_OPTS) -std=c++11 -DGALILEO_GEN=${GALILEO_GEN}
CPPFLAGS    := $(COMPILE_OPTS) -std=c++11 -DGALILEO_GEN=${GALILEO_GEN}

