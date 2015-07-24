# For CLion see http://stackoverflow.com/questions/26918459/using-local-makefile-for-clion-instead-of-cmake

cmake_minimum_required(VERSION 3.2)
project(galileo_terremoti)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

# set(CMAKE_VERBOSE_MAKEFILE ON)

add_subdirectory(arduino)

link_directories(build)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/build/")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/build/")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/build/")

set(SOURCE_FILES
	galileo-core.cpp
	avg.h
	Config.h
	Log.h
	net/NTP.h
	net/HTTPClient.h
	AcceleroMMA7361.cpp
	AcceleroMMA7361.h
	avg.cpp
	Log.cpp
	Config.cpp
	net/NTP.cpp
	net/HTTPClient.cpp
	net/NetworkManager.cpp
	net/NetworkManager.h
	LED.h
	LED.cpp
	Seismometer.cpp
	Seismometer.h
	Utils.cpp
	Utils.h
	CommandInterface.cpp
	CommandInterface.h
	common.h
	)

add_executable(galileo_gen1 ${SOURCE_FILES})
target_compile_options(galileo_gen1 PUBLIC "-DGALILEO_GEN=1")
target_include_directories(galileo_gen1 PUBLIC ${GALILEO_GEN1_INCLUDE})
target_link_libraries(galileo_gen1 ArduinoGalileo1 ArduinoGalileo1Ethernet)

add_executable(galileo_gen2 ${SOURCE_FILES})
target_compile_options(galileo_gen2 PUBLIC "-DGALILEO_GEN=2")
target_include_directories(galileo_gen2 PUBLIC ${GALILEO_GEN2_INCLUDE})
target_link_libraries(galileo_gen2 ArduinoGalileo2 ArduinoGalileo2Ethernet)

add_custom_target(gen1image COMMAND genimage.sh ${PROJECT_SOURCE_DIR})
