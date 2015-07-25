# Requirements

* Galileo toolchain (see below)
* cmake
* GNU/Linux

# Toolchains
## Galileo

To create a toolchain for Galileo:

1. Download Galileo Arduino IDE from Intel website
2. Extract archive to /opt/ (so you have /opt/arduino-1.6.0+Intel/arduino executable)

TODO: If you don't have root access, you can place these files to `$HOME/.arduino-toolchain`

`CMakeToolchain.cmake` is based on Makefile at https://github.com/tokoro10g/galileo-makefile

# How to build Arduino gen1/gen2 image from command line

Just use `cmake -DCMAKE_TOOLCHAIN_FILE=CMakeToolchain.cmake` and then `make`

# How to build Arduino gen1/gen2 image from CLion

Open File menu -> Settings -> Build, Execution, Deployment -> CMake -> enter `-DCMAKE_TOOLCHAIN_FILE=CMakeToolchain.cmake`
as CMake option.

You'll need a reload of cmake cache: open File menu -> Reload CMake Project. If doesn't work, remove cache directory under
`$HOME/.clion10/system/cmake/generated/` and click "Reload CMake project" again

# How to build Arduino Galileo SD Image (Linux-only)

TODO: update this doc

Just use `make gen1image` or `make gen2image` to build and generate files for Arduino Galileo SD Card (based on GNU/Linux
code provided by Intel).

Files will be placed on `build/image-full-galileo/` (copy contents to an empty FAT32-formatted SD Card).

# How to contribute

You can use any IDE or text editor. Just use `cmake -DCMAKE_TOOLCHAIN_FILE=CMakeToolchain.cmake` to compile.

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