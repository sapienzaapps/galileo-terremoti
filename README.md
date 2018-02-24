[![Build Status](https://travis-ci.org/sapienzaapps/galileo-terremoti.svg?branch=master)](https://travis-ci.org/sapienzaapps/galileo-terremoti)

# NodeMCU?

To use NodeMCU boards you must use sketch at https://github.com/sapienzaapps/seismoclouddevice-nodemcu . The code here is for Intel Galileo / Raspberry only.

# License

See LICENSE file

# Requirements

* GNU/Linux or OSX (others \*BSD are not supported but it should works)
* GNU make
* Galileo toolchain (if you plan to compile for Arduino Galileo), you can find it here: http://www.intel.com/support/galileo/sb/CS-035101.htm
* GCC compiler (if you plan to compile for `linux-x86`)
* Raspberry PI cross-compilation toolchain (if you plan to compile for `raspi`)

## Network requirements

If you have any firewall in your network, please allow these ports to Internet (outbound):

* TCP: 80, 443, 1883
* UDP: 123

# Hardware
## Hardware needed

* 3 LEDs (Red, Green, Blue) with 3 resistor
* 1 Accelerometer (MMA7361 if you have analog inputs, ADXL345 for i2c)
* Arduino Galileo / Raspberry PI
* Donuts cables
* Ethernet wired connection

If you plan to use Arduino Uno or Arduino Galileo you should use MMA7361 accelerometer and analog pins on-board. If you use Raspberry, you should choose ADXL345 accelerometer instead (because Raspberry PI uses I2C bus to communicate with ADXL345).

**Raspberry PI users**: if `i2c` bus is not enabled, please refer to `Platform specific` chapter of this README to enable i2c on Raspberry PI.

## LEDs

LEDs can be in these different states:

* **Green**: device is ready
* **Green + Yellow**: device is ready but there is an issue connecting to SeismoCloud APIs
* **Green (still) + Yellow (blinking)**: device is calibrating
* **Green + Red (only for about 5 seconds)**: shake detected
* **Yellow ONLY blinking**: no position available - initialize Seismometer with Android/iOS App
* **Red ONLY** still: reboot/upgrade in progress
* **Green + Yellow + Red - ALL rotating**: software is loading
* **Green + Yellow + Red - ALL blinking fast**: software is loaded, starting accelerometer

### LED pins on Arduino Galileo

* Pin 8 : Yellow
* Pin 10 : Green
* Pin 12 : Red

### LED pins on Raspberry PI

* GPIO-17 (wiringpi addr #0) : Green
* GPIO-18 (wiringpi addr #1) : Yellow
* GPIO-21 (wiringpi addr #2) : Red

## Link MMA7361 Accelerometer to Galileo

Link these pins from Accelerometer MMA7361 to Arduino Galileo board:

* Vin: 5v
* GND: GND
* SEL: GND
* X: A0
* Y: A1
* Z: A2

Loop back **3v3** pin to **SLP** on Accelerometer.

**Note**: the accelerometer should be aligned with Z perpendicular to the ground.

## Link ADXL345 Accelerometer to Raspberry PI

* 3.3v : 3.3v
* GND : GND
* SDA : SDA
* SCL : SCL

Refer to Raspberry PI pinout, as https://jeffskinnerbox.files.wordpress.com/2012/11/raspberry-pi-rev-1-gpio-pin-out1.jpg

# Setup your build environment
## Cross compile for Galileo

### on OS X

You should have Arduino Galileo IDE installed into `/Applications`. If not you can use the `GNU/Linux` guide to create a toolchain in a custom path.

#### on GNU/Linux

To create a toolchain for Galileo:

1. Download Galileo Arduino IDE from Intel website
2. Extract archive to `/opt/` (so you'll have `/opt/arduino-1.6.0+Intel/arduino` executable)

If you don't have root access, you can place these files to any path you like (remember to use **SDK_ROOT** make option to specify different path)

## Compile on Raspberry Pi (Raspbian Jessie)

Make sure that you have these (debian) packages: `build-essential git wiringpi` and you're good to go.

**Note**: you need to remove `libi2c-dev` if you have it because there are compatibility issues with some headers (you need to use i2c headers that are bundled with Linux kernel headers/source).

# How to build from command line

You should issue `make` command into project root directory. Make targets availables are: `all`, `clean` and `upload` (see below).

You may use these options to compile a particular version:

* **PLATFORM** : can be `linux-x86`, `mac-osx`, `raspi` or `galileo` depending on your target system
* **VARIANT** : platform variant; `galileo` has two "variants":
	* **galileo_fab_d** : Galileo Gen 1 (default)
	* **galileo_fab_g** : Galileo Gen 2
* **SDK_ROOT** : SDK/Toolchain root path (see "Toolchains") if it's not in default paths (`/opt` for GNU/Linux, /Applications for OS X)
* **DEBUG** : if nonempty, enables debug options (i.e. debug messages and commands, crash reports)
* **DEBUG_SERVER** : if nonempty, device will use testing APIs
* **NOWATCHDOG** : if nonempty, watchdog will be disabled
* **REMOTEHOST** : if nonempty, enables `upload` target (Galileo only)

## First time compilation with Arduino Galileo toolchain

On first time you need to use Arduino Galileo IDE to "relocate" (eg. setting up some paths).
To do so, please open Arduino IDE, select "Arduino Galileo" board from menu and click on "verify/compile" (tick icon).

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

See `linux-x86`, `mac-osx` and `galileo` for more infos.

# Platform specific informations

## Linux

In order to test latency you need to run `sketch.elf` as root OR grant `CAP_NET_RAW` capability with a command like:

    $ sudo setcap cap_net_raw=ep build/out_linux-x86/sketch.elf

## I2C bus not available on latest Raspbian releases

Please refer to this post: https://www.raspberrypi.org/forums/viewtopic.php?f=28&t=97314

In short, you should run `raspi-config` and enable `i2c` under `Advanced Options` menu (and then reboot).

## GDB Debug (cross)

To debug core dumps from Galileo you need "gdb-multiarch" (usually shipped in your distro's repos). Example:

    $ gdb-multiarch
    ...
    This GDB was configured as "x86_64-linux-gnu".
    Type "show configuration" for configuration details.
    For bug reporting instructions, please see:
    <http://www.gnu.org/software/gdb/bugs/>.
    ...
    (gdb) set gnutarget elf32-littlearm
    (gdb) file sketch.elf
    (gdb) target core coredump.dat
    (gdb) set sysroot /opt/arduino-1.6.0+Intel/hardware/tools/i586/sysroots/i586-poky-linux-uclibc/
    (gdb) bt

## Galileo: disable connmand (static IP)

To disable `connmand` and revert plain old `networking` script, use these commands:

    # rm /etc/rc5.d/S05connman
    # ln -s ../init.d/networking /etc/rc5.d/S05networking
