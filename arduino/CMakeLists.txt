cmake_minimum_required(VERSION 3.2)
project(ArduinoLibraries)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/build/")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/build/")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/build/")

file(GLOB SOURCE_FILES_CPP ${ARDUINO_IDE_ROOT}/hardware/intel/i586-uclibc/cores/arduino/*.cpp)
file(GLOB SOURCE_FILES_C ${ARDUINO_IDE_ROOT}/hardware/intel/i586-uclibc/cores/arduino/*.c)

file(GLOB GALILEO_GEN1_SRC ${GALILEO_GEN1_INCLUDE}/*.cpp)
file(GLOB GALILEO_GEN2_SRC ${GALILEO_GEN2_INCLUDE}/*.cpp)

include_directories(${ARDUINO_IDE_ROOT}/hardware/intel/i586-uclibc/cores/arduino/)

add_library(ArduinoGalileo1 STATIC ${SOURCE_FILES_CPP} ${SOURCE_FILES_C} ${GALILEO_GEN1_SRC})
target_include_directories(ArduinoGalileo1 PUBLIC ${GALILEO_GEN1_INCLUDE})
target_link_libraries(ArduinoGalileo1 pthread)

add_library(ArduinoGalileo2 STATIC ${SOURCE_FILES_CPP} ${SOURCE_FILES_C} ${GALILEO_GEN2_SRC})
target_include_directories(ArduinoGalileo2 PUBLIC ${GALILEO_GEN2_INCLUDE})
target_link_libraries(ArduinoGalileo2 pthread)



file(GLOB ETH_SRC ${ARDUINO_IDE_ROOT}/hardware/intel/i586-uclibc/libraries/Ethernet/src/*.*)

add_library(ArduinoGalileo1Ethernet STATIC ${ETH_SRC})
target_include_directories(ArduinoGalileo1 PUBLIC ${GALILEO_GEN1_INCLUDE})
target_link_libraries(ArduinoGalileo1Ethernet ArduinoGalileo1)

add_library(ArduinoGalileo2Ethernet STATIC ${ETH_SRC})
target_include_directories(ArduinoGalileo2 PUBLIC ${GALILEO_GEN2_INCLUDE})
target_link_libraries(ArduinoGalileo2Ethernet ArduinoGalileo2)