# Requirements

* Arduino IDE
* make (optional)

# How to compile using Arduino IDE

If you have GNU/Linux or OSX (and `make`): just create a file named `buildcfg.h` by doing `make gen1prep` or `make gen2prep` and then launch Arduino Galileo IDE. You can change it later by re-issuing one of these commands.

Alternatively (eg. if you're on Windows or you don't have `make`), you can create that file manually with this content:

    #ifndef __BUILDCFG_H
    #define __BUILDCFG_H

    #define __IS_GALILEO
    // If you have Gen2 Galileo please set GALILEO_GEN to 2
    #define GALILEO_GEN 1

    #endif

# How to compile from command line (Linux-only)

**IMPORTANT**: You should have `arduino` command (Arduino Galileo IDE) in your `$PATH`. If not, you can issue the following command (replace /arduino/ide/path/ with **absolute** path to Arduino IDE folder)

    # export PATH=$PATH:/arduino/ide/path/

Enter `make gen1` to make Arduino Galileo Gen1 sketch file, or `make gen2` to make Arduino Galileo Gen2 sketch file. Compilation output will be `build/galileo-terremoti.cpp.elf`.

If you want upload sketch to Arduino Galileo, you can use `make gen1upload` or `make gen2upload`.
Note that by default Arduino tools will try to upload to `/dev/ttyACM0`: if your Galileo is on a different port, set `$ARDUINODEV` environment variable.

# How to compile using CLion (Linux-only)

To use CLion you should have an environment variable named `$ARDUINOIDE` pointing to Arduino Galileo IDE root directory (eg. the directory which contains "arduino" IDE executable).

For example:

	# export ARDUINOIDE=/home/bob/arduino-galileo-ide/
	# cd clion-1.0.4/bin/
	# ./clion.sh

(alternatively you can add `export` command to `.bashrc` or directly into `/etc/environment`)

# How to make Arduino Galileo SD Image (Linux-only)

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