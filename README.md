# Requirements

* GNU/Linux
* GNU make
* Galileo toolchain (if you plan to compile for Arduino), you can find it here: http://www.intel.com/support/galileo/sb/CS-035101.htm
* GCC compiler (if you plan to compile for linux-x86)

# Toolchains
## Galileo

To create a toolchain for Galileo:

1. Download Galileo Arduino IDE from Intel website
2. Extract archive to /opt/ (so you have /opt/arduino-1.6.0+Intel/arduino executable)

TODO: If you don't have root access, you can place these files to `$HOME/.arduino-toolchain`

# How to build from command line

You should issue `make` command into project root directory. You should use these options to compile a particular version:

* **PLATFORM** : can be `linux-x86` (default) or `galileo`
* **VARIANT** : platform variant; for "galileo" variants are:
	* **galileo_fab_d** : Galileo Gen 1
	* **galileo_fab_g** : Galileo Gen 2
* **SDK_ROOT** : SDK root path (see "Toolchains") if it's not in `/opt`

# How to build Arduino Galileo SD Image (Linux-only)

TODO: update this doc

Just use `make gen1image` or `make gen2image` to build and generate files for Arduino Galileo SD Card (based on GNU/Linux
code provided by Intel).

Files will be placed on `build/image-full-galileo/` (copy contents to an empty FAT32-formatted SD Card).

# How to add a new platform

Project code is written as much generic as possible. Platform-specific code is placed into vendor/ directory.
Specific code includes: LED control, 3-axis sensor code, etc.

You should implement these classes as a .cpp file into platform specific directory (example: `vendor/linux-x86/LED.cpp`):
* LED class
* generic.cpp

You'll find definitions into .h files in project root directory.
Also you may need to create a child class of Accelerometer.h and a vendor_specific.h into vendor directory.

See `linux-x86` and `galileo` for more infos.

# Platform specific informations

## Linux

In order to test latency you need to run `sketch.elf` as root OR grant `CAP_NET_RAW` capability with a command like:

    $ sudo setcap cap_net_raw=ep build/out_linux-x86/sketch.elf