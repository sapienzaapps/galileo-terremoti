
include(CMakeForceCompiler)

# Based on https://github.com/tokoro10g/galileo-makefile

# this one is important
SET(CMAKE_SYSTEM_NAME Linux)
#this one not so much
SET(CMAKE_SYSTEM_VERSION 1)

# specify the cross compiler
set(DISTRO clanton-tiny)
set(TARGET_NAME i586-poky-linux-uclibc)
set(NATIVE_NAME x86_64-pokysdk-linux)
set(GCC_VERSION 4.7.2)

set(ARDUINO_IDE_ROOT /opt/arduino-1.6.0+Intel/)

# TODO: use $HOME/.arduino-toolchain if exists

set(SYSROOT_TARGET ${ARDUINO_IDE_ROOT}/hardware/tools/i586/sysroots/${TARGET_NAME}/)
set(SYSROOT_NATIVE ${ARDUINO_IDE_ROOT}/hardware/tools/i586/sysroots/${NATIVE_NAME}/)

set(TARGET_ARCH "-m32 -march=i586 --sysroot=${SYSROOT_TARGET}")

set(TOOLDIR ${SYSROOT_NATIVE}/usr/bin/${TARGET_NAME})
set(CC ${TOOLDIR}/i586-poky-linux-uclibc-gcc)
set(CCC ${TOOLDIR}/i586-poky-linux-uclibc-g++)

set(GALILEO_VARIANTS ${ARDUINO_IDE_ROOT}/hardware/intel/i586-uclibc/variants/)
set(GALILEO_GEN1_INCLUDE ${GALILEO_VARIANTS}/galileo_fab_d/)
set(GALILEO_GEN2_INCLUDE ${GALILEO_VARIANTS}/galileo_fab_g/)

include_directories(BEFORE ${ARDUINO_IDE_ROOT}/hardware/intel/i586-uclibc/cores/arduino/)
include_directories(BEFORE ${ARDUINO_IDE_ROOT}/hardware/intel/i586-uclibc/libraries/Ethernet/src/)
include_directories(BEFORE ${ARDUINO_IDE_ROOT}/hardware/intel/i586-uclibc/libraries/SPI/src/)
include_directories(BEFORE ${SYSROOT_TARGET}/usr/include)
include_directories(BEFORE ${SYSROOT_TARGET}/usr/include/c++)
include_directories(BEFORE ${SYSROOT_TARGET}/usr/include/c++/${TARGET_NAME}) # <--
include_directories(BEFORE ${SYSROOT_NATIVE}/usr/lib/${TARGET_NAME}/gcc/${TARGET_NAME}/${GCC_VERSION}/include)

link_directories(${SYSROOT_TARGET}/lib)
link_directories(${SYSROOT_TARGET}/usr/lib)
link_directories(${SYSROOT_TARGET}/usr/lib/${TARGET_NAME}/${GCC_VERSION})

#file(GLOB Ethernet_Source ${ARDUINO_IDE_ROOT}/hardware/intel/i586-uclibc/libraries/Ethernet/src/*.cpp)
#file(GLOB Ethernet_Headers ${ARDUINO_IDE_ROOT}/hardware/intel/i586-uclibc/libraries/Ethernet/src/*.h)

#add_library(ArduinoGalileo1Ethernet STATIC EXCLUDE_FROM_ALL ${Ethernet_Source} ${Ethernet_Headers})
#target_link_libraries(ArduinoGalileo1Ethernet ArduinoGalileo1)
#
#add_library(ArduinoGalileo2Ethernet STATIC EXCLUDE_FROM_ALL ${Ethernet_Source} ${Ethernet_Headers})
#target_link_libraries(ArduinoGalileo2Ethernet ArduinoGalileo2)



CMAKE_FORCE_C_COMPILER(${CC} GNU)
CMAKE_FORCE_CXX_COMPILER(${CCC} GNU)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -pipe -g -feliminate-unused-debug-types -fpermissive -Wall -Wl,-O1 -Wl,--hash-style=gnu -Wl,--as-needed -lstdc++ ${TARGET_ARCH}")

# where is the target environment 
SET(CMAKE_FIND_ROOT_PATH  ${SYSROOT_TARGET})

# search for programs in the build host directories
SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
# for libraries and headers in the target directories
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)