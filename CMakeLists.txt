# For CLion see http://stackoverflow.com/questions/26918459/using-local-makefile-for-clion-instead-of-cmake

cmake_minimum_required(VERSION 3.2)
project(galileo_terremoti)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

include_directories($ENV{ARDUINOIDE}/hardware/intel/i586-uclibc/cores/arduino/)
include_directories($ENV{ARDUINOIDE}/libraries/Ethernet/src/)
include_directories($ENV{ARDUINOIDE}/libraries/SD/src/)
include_directories($ENV{ARDUINOIDE}/hardware/intel/i586-uclibc/libraries/SPI/src/)
include_directories($ENV{ARDUINOIDE}/hardware/intel/i586-uclibc/libraries/EEPROM/src/)

# Gen1 - used only for code completion - no need to change between gen1-gen2
include_directories($ENV{ARDUINOIDE}/hardware/intel/i586-uclibc/variants/galileo_fab_d/)
# Gen2
#include_directories(galileo_gen2 PUBLIC $ENV{ARDUINOIDE}/hardware/intel/i586-uclibc/variants/galileo_fab_g/)

set(SOURCE_FILES galileo-core.cpp
	avg.h
	commons.h
	config.h
	GalileoLog.h
	ntp_alt.h
	threshold.h
	localstream.h
	httpconn.h
	cfgupdate.h
	AcceleroMMA7361.cpp
	AcceleroMMA7361.h
	buildcfg.h
	)
add_executable(galileo_terremoti ${SOURCE_FILES})

add_custom_target(galileo_gen1 COMMAND make gen1 -C ${PROJECT_SOURCE_DIR}
	CLION_EXE_DIR=${PROJECT_BINARY_DIR}
	SOURCES ${SOURCE_FILES})

add_custom_target(galileo_gen1upload COMMAND make gen1upload -C ${PROJECT_SOURCE_DIR}
	CLION_EXE_DIR=${PROJECT_BINARY_DIR}
	SOURCES ${SOURCE_FILES})

add_custom_target(galileo_gen2 COMMAND make gen2 -C ${PROJECT_SOURCE_DIR}
	CLION_EXE_DIR=${PROJECT_BINARY_DIR}
	SOURCES ${SOURCE_FILES})

add_custom_target(galileo_gen2upload COMMAND make gen2upload -C ${PROJECT_SOURCE_DIR}
	CLION_EXE_DIR=${PROJECT_BINARY_DIR}
	SOURCES ${SOURCE_FILES})
