
# Default platform is running OS

UNAME_S := $(shell uname -s)

ifeq ($(UNAME_S),Darwin)
PLATFORM ?= mac-osx
else
LSB_RELEASE := $(shell lsb_release -i -s)
ifeq ($(LSB_RELEASE),Raspbian)
PLATFORM ?= raspi
else
# Raspian Wheezy doesn't show Raspbian on LSB, so we're guessing from machine hardware
HWPLATFORM := $(shell uname -m)
ifeq ($(HWPLATFORM),armv6l)
PLATFORM ?= raspi
else
PLATFORM ?= linux-x86
endif
endif
endif

