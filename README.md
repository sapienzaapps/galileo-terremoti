# Requirements

* GNU/Linux or OSX (others \*BSD are not supported but it should works)
* GNU make
* Galileo toolchain (if you plan to compile for Arduino), you can find it here: http://www.intel.com/support/galileo/sb/CS-035101.htm
* GCC compiler (if you plan to compile for linux-x86)

# LED outputs

LEDs can be in these different states:

* **Green**: device is ready
* **Green + Yellow**: device is ready but there is an issue connecting to SeismoCloud APIs
* **Green (still) + Yellow (blinking)**: device is calibrating
* **Green + Red (only for about 5 seconds)**: shake detected
* **Yellow ONLY blinking**: no position available - initialize Seismometer with Android/iOS App
* **Red ONLY** still: reboot/upgrade in progress
* **Green + Yellow + Red - ALL rotating**: software is loading
* **Green + Yellow + Red - ALL blinking fast**: software is loaded, starting accelerometer

# Toolchains
## Galileo

## OS X

You should have Arduino Galileo IDE installed into `/Applications`. If not you can use the `GNU/Linux` guide to create a toolchain in a custom path.

### GNU/Linux

To create a toolchain for Galileo:

1. Download Galileo Arduino IDE from Intel website
2. Extract archive to `/opt/` (so you'll have `/opt/arduino-1.6.0+Intel/arduino` executable)

If you don't have root access, you can place these files to any path you like (remember to use **SDK_ROOT** make option to specify different path)

# How to build from command line

You should issue `make` command into project root directory. You should use these options to compile a particular version:

* **PLATFORM** : can be `linux-x86` (default) or `galileo`
* **VARIANT** : platform variant; for "galileo" variants are:
	* **galileo_fab_d** : Galileo Gen 1 (default)
	* **galileo_fab_g** : Galileo Gen 2
* **SDK_ROOT** : SDK root path (see "Toolchains") if it's not in `/opt` (or /Applications for OS X)
* **DEBUG** : if set, enables debug options (i.e. debug messages and commands)
* **DEBUG_URL** : if set, device will use testing APIs

# How to build Arduino Galileo SD Image (Linux-only)

Just use `./genimage.sh build/path-to-sketch-depending-on-vendor/sketch.elf`

Files will be placed on `build/image-full-galileo/` (copy contents to an empty FAT32-formatted SD Card).

# How to add a new platform

Project code is written as much generic as possible. Platform-specific code is placed into `vendor/` directory.
Specific code includes: LED control, 3-axis sensor code, etc.

You should implement these classes as a .cpp file into platform specific directory (example: `vendor/linux-x86/LED.cpp`):
* `LED class`
* `generic.cpp`

You'll find definitions into `.h` files in project root directory.
Also you may need to create a child class of `Accelerometer.h` and a `vendor_specific.h` into vendor directory.

See `linux-x86` and `galileo` for more infos.

# Platform specific informations

## Linux

In order to test latency you need to run `sketch.elf` as root OR grant `CAP_NET_RAW` capability with a command like:

    $ sudo setcap cap_net_raw=ep build/out_linux-x86/sketch.elf


