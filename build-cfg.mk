
# Default platform is running OS

UNAME_S := $(shell uname -s)
LSB_RELEASE := $(shell lsb_release -i -s)

ifeq ($(UNAME_S),Darwin)
PLATFORM ?= mac-osx
else
ifeq ($(LSB_RELEASE),Raspbian)
PLATFORM ?= raspi
else
PLATFORM ?= linux-x86
endif
endif

