# Requirements

* Arduino IDE
* make (optional)

# How to compile using Arduino IDE

If you have GNU/Linux or OSX (and `make`): just create a file named `buildcfg.h` by doing `make gen1prep` or `make gen2prep`. You can change it later by re-issuing one of these commands.

Alternatively (eg. if you're on Windows or you don't have `make`), you can create that file manually with this content:

    #ifndef __BUILDCFG_H
    #define __BUILDCFG_H

    #define __IS_GALILEO
    // If you have Gen2 Galileo please set GEN1 to 0 and GEN2 to 1
    #define GEN1 1
    #define GEN2 0

    #endif

# How to compile from command line (Linux-only)

**IMPORTANT**: You should have `arduino` command (Arduino Galileo IDE) in your `$PATH`. If not, you can issue the following command (replace /arduino/ide/path/ with **absolute** path to Arduino IDE folder)

    # export PATH=$PATH:/arduino/ide/path/

Enter `make gen1` to make Arduino Galileo Gen1 sketch file, or `make gen2` to make Arduino Galileo Gen2 sketch file. Compilation output will be `build/galileo-terremoti.cpp.elf`.

If you want upload sketch to Arduino Galileo, you can use `make gen1upload` or `make gen2upload`.
Note that by default Arduino tools will try to upload to `/dev/ttyACM0`: if your Galileo is on a different port, set `$ARDUINODEV` environment variable.
