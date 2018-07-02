[![Build Status](https://travis-ci.org/sapienzaapps/galileo-terremoti.svg?branch=master)](https://travis-ci.org/sapienzaapps/galileo-terremoti)

**Intel Galileo support is dropped**, so please choose **Raspberry PI** or **NodeMCU**.

# NodeMCU?

To use NodeMCU boards you must use sketch at https://github.com/sapienzaapps/seismoclouddevice-nodemcu . The code here is for Raspberry PI only.

# License

See LICENSE file

# Requirements

* GNU/Linux or OSX (others \*BSD are not supported but it should works)
* GNU make
* GCC compiler (if you plan to compile for `linux-x86`)
* Raspberry PI cross-compilation toolchain (if you plan to compile for `raspi`)

## Network requirements

If you have any firewall in your network, please allow these ports to Internet (outbound):

* TCP: 80, 443, 1883
* UDP: 123

# Hardware
## Hardware needed

* 3 LEDs (Red, Green, Blue) with 3 resistor
* 1 Accelerometer (ADXL345)
* Raspberry PI
* Donuts cables
* Ethernet wired connection

**Important**: if `i2c` bus is not enabled, please refer to `Platform specific` chapter of this README to enable i2c on Raspberry PI.

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

### LED pins on Raspberry PI

* GPIO-17 (wiringpi addr #0) : Green
* GPIO-18 (wiringpi addr #1) : Yellow
* GPIO-21 (wiringpi addr #2) : Red

## Link ADXL345 Accelerometer to Raspberry PI

* 3.3v : 3.3v
* GND : GND
* SDA : SDA
* SCL : SCL

Refer to Raspberry PI pinout, as https://jeffskinnerbox.files.wordpress.com/2012/11/raspberry-pi-rev-1-gpio-pin-out1.jpg

# Setup your build environment
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
