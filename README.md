# Requirements

* Galileo toolchain (see below)
* cmake
* GNU/Linux

# Toolchains
## Galileo

To create a toolchain for Galileo:

1. Download Galileo Arduino IDE from Intel website
2. Extract archive
3. Copy toolchain:
```
$ cd /where/you/extracted/the/archive/arduino-1.6.0+Intel/
$ cd hardware/tools/i586/sysroots
$ sudo mkdir -p /opt/clanton-tiny/1.4.2/sysroots
$ sudo cp -R hardware/tools/i586/sysroots/i586-poky-linux-uclibc /opt/clanton-tiny/1.4.2/sysroots/
$ sudo cp -R hardware/tools/i586/sysroots/x86_64-pokysdk-linux /opt/clanton-tiny/1.4.2/sysroots/
$ sudo cp -R hardware/intel/i586-uclibc/ /opt/clanton-tiny/1.4.2/sysroots/i586-poky-linux-uclibc/arduino
$ sudo cp -R hardware/intel/i586-uclibc/ /opt/clanton-tiny/1.4.2/sysroots/x86_64-pokysdk-linux/arduino
```
It copies cross devtools from Arduino IDE. If you don't have root access, you can place these files to `$HOME/.arduino-toolchain`

`CMakeToolchain.cmake` is based on Makefile at https://github.com/tokoro10g/galileo-makefile

# How to contribute

You can use any IDE or text editor. Just use `cmake -DCMAKE_TOOLCHAIN_FILE=CMakeToolchain.cmake` to compile.

# How to make Arduino Galileo SD Image (Linux-only)

TODO: update this doc

Just use `make gen1image` or `make gen2image` to build and generate files for Arduino Galileo SD Card (based on GNU/Linux code provided by Intel).

Files will be placed on `build/image-full-galileo/` (copy contents to an empty FAT32-formatted SD Card).

# Working procedure

## Boot

1. Enabling file logging to level DEBUG
2. Setting analog read resolution to 10 (3.3V => 4096)
3. Fixing Galileo bugs on network sockets (networking restart and register SIGPIPE to SIG_IGN)
4. Loading config file
5. Initial Accelerometer calibration
6. Setup networking
7. Enabling syslog
8. If no MAC available, ask to server
9. start = Config::hasPosition()
10. Some leds stuffs (network available and others)
11. If internet and position are OK, get config from server
12. Startup local network command UDP socket
13. If internet is connected, sync with NTP
14. If reboot script doesn't exists, create it

## Loop

1. At `checkInternetConnectionInterval`, check internet connection. If not OK, enable "reset ethernet" flag
2. At `NTPInterval`, sync with NTP server
3. If `ForceCalibrationNeeded` or at `calibrationInterval` force check for calibration
4. If we have generated too many alerts (`numAlert >= reTareNum`), force recalibration
5. Reset `inEvent` after `nextContact` time
6. If we need to reset ethernet, then reset NOW!
7. At every 50UL ms and if `zz < 3` (unknown) do
	1. If we don't need to record values, then do normal sensor detection
	2. If we need to record values, then start if not started, and record
8. At `checkConfigInterval` check new config from server
9. Then after 24h or 20 errors, reset/reboot Galileo