//
// Created by ebassetti on 23/07/15.
//

#ifndef GALILEO_TERREMOTI_COMMON_H
#define GALILEO_TERREMOTI_COMMON_H

#define MINVAL(a, b) ((a) < (b) ? (a) : (b))

#ifndef PLATFORM
#define PLATFORM "unknown"
#endif
#ifndef BUILD_VERSION
#define BUILD_VERSION "unknown"
#endif

#include <unistd.h>
#include <stdint.h>
#include <vendor_specific.h>

typedef uint8_t byte;

#define SOFTWARE_VERSION "2.15"

#define NTP_SYNC_INTERVAL            60*60*1000
#define CHECK_NETWORK_INTERVAL       30*1000  // when to check for Internet connection availability
#define ALIVE_INTERVAL               15*60*1000  // Alive timeout
#define SEISMOMETER_TICK_INTERVAL    50
#define HTTP_RESPONSE_TIMEOUT_VALUE  10*1000
#define WATCHDOG_TIMER               60*1000

typedef struct {
	unsigned long ts;
	double accel;
	bool overThreshold;
} RECORD;

#ifndef DEBUG_SERVER
#define MQTT_SERVER  "mqtt.seismocloud.com"
#define MQTT_PORT    1883
#define HTTP_HOST    "www.sapienzaapps.it"
#define HTTP_PATH    "/seismocloud/"
#else
#define MQTT_SERVER  "10.10.10.1"
#define MQTT_PORT    1883
#define HTTP_HOST    "10.10.10.1"
#define HTTP_PATH    "/"
#endif

#define HTTP_BASE    std::string("http://") + HTTP_HOST + HTTP_PATH

#endif //GALILEO_TERREMOTI_COMMON_H
