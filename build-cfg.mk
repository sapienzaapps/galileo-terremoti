
# Default platform is running OS

UNAME_S := $(shell uname -s)

ifeq ($(UNAME_S),Darwin)
PLATFORM ?= mac-osx
else
PLATFORM ?= linux-x86
endif

