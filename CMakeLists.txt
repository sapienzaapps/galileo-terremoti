# For CLion see http://stackoverflow.com/questions/26918459/using-local-makefile-for-clion-instead-of-cmake

cmake_minimum_required(VERSION 3.2)
project(galileo_terremoti)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
add_Definitions(-DDEBUG)
add_Definitions(-DSDL_DEMO)

#set(CMAKE_VERBOSE_MAKEFILE ON)

#add_subdirectory(arduino)

#link_directories(build)

#set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/build/")
#set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/build/")
#set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/build/")

set(SOURCE_FILES
		galileo-core.cpp
		Config.h
		Log.h
        net/HTTPClient.h
		Log.cpp
		Config.cpp
        net/HTTPClient.cpp
		LED.h
		Seismometer.cpp
		Seismometer.h
		Utils.cpp
		Utils.h
		CommandInterface.cpp
		CommandInterface.h
		common.h
		net/IPaddr.cpp
		net/IPaddr.h
		net/Udp.cpp
		net/Udp.h
		net/Tcp.cpp
		net/Tcp.h
		net/IPaddr.h
		net/IPaddr.cpp
		Accelerometer.h
		vendor/galileo/AcceleroMMA7361.cpp
		vendor/galileo/AcceleroMMA7361.h
		vendor/galileo/LED.cpp
        vendor/galileo/generic.cpp
        vendor/galileo/vendor_specific.h
		vendor/linux-x86/DummyAccelerometer.cpp
		vendor/linux-x86/DummyAccelerometer.h
		vendor/linux-x86/LED.cpp
		vendor/linux-x86/generic.cpp
		vendor/linux-x86/vendor_specific.h
        LCDMonitor.h
        LCDMonitor.cpp
        vendor/mac-osx/DummyAccelerometer.cpp
        vendor/mac-osx/DummyAccelerometer.h
        vendor/mac-osx/LED.cpp
        vendor/mac-osx/generic.cpp
        vendor/mac-osx/vendor_specific.h
		vendor/raspi/ADXL345Accelerometer.cpp
		vendor/raspi/ADXL345Accelerometer.h
		vendor/raspi/LED.cpp
		vendor/raspi/generic.cpp
		vendor/raspi/smbus.cpp
		vendor/raspi/smbus.h
		vendor/raspi/vendor_specific.h
        generic.h
		Watchdog.cpp
        Watchdog.h
		net/TraceAccumulator.cpp net/TraceAccumulator.h
        net/MQTT.cpp net/MQTT.h
		net/SCSAPI_MQTT.cpp
		net/SCSAPI.h
		net/SCSAPI_MQTT.h
		net/SCSAPI_HTTP.cpp
		net/SCSAPI_HTTP.h
		base64.cpp
		base64.h
		net/WebSocket.h
		net/WebSocket.cpp
		net/MQTT_Subscribe.cpp
		net/MQTT_Subscribe.h
		net/DataStream.cpp
		net/DataStream.h)

add_executable(galileo_gen1 EXCLUDE_FROM_ALL ${SOURCE_FILES})
#target_compile_options(galileo_gen1 PUBLIC "-DGALILEO_GEN=1")
#target_include_directories(galileo_gen1 PUBLIC ${GALILEO_GEN1_INCLUDE})
#target_link_libraries(galileo_gen1 ArduinoGalileo1 ArduinoGalileo1Ethernet)

#add_executable(galileo_gen2 ${SOURCE_FILES})
#target_compile_options(galileo_gen2 PUBLIC "-DGALILEO_GEN=2")
#target_include_directories(galileo_gen2 PUBLIC ${GALILEO_GEN2_INCLUDE})
#target_link_libraries(galileo_gen2 ArduinoGalileo2 ArduinoGalileo2Ethernet)

#add_custom_target(gen1image COMMAND genimage.sh ${PROJECT_SOURCE_DIR})

include_directories(vendor/linux-x86/)
include_directories($ENV{HOME}/homebrew/Cellar/sdl2/2.0.3/include)

add_custom_target(generic_x86 ALL
		COMMAND make PLATFORM="linux-x86" -C ${PROJECT_SOURCE_DIR}
		SOURCES ${SOURCE_FILES})

add_custom_target(galileo1 ALL
		COMMAND make PLATFORM="galileo" VARIANT="galileo_fab_d" -C ${PROJECT_SOURCE_DIR}
		SOURCES ${SOURCE_FILES})

add_custom_target(galileo2 ALL
		COMMAND make PLATFORM="galileo" VARIANT="galileo_fab_g" -C ${PROJECT_SOURCE_DIR}
		SOURCES ${SOURCE_FILES})

add_custom_target(clear ALL
		COMMAND make clean -C ${PROJECT_SOURCE_DIR}
		SOURCES ${SOURCE_FILES})
