//
// Created by ebassetti on 23/07/15.
//

#ifndef GALILEO_TERREMOTI_COMMON_H
#define GALILEO_TERREMOTI_COMMON_H

#include <BitsAndBytes.h>

#define DEFAULT_LOG_PATH "/media/realroot/sketch.log"
#define DEFAULT_CONFIG_PATH "/media/realroot/seismoconfig.txt"
#define GFORCE 9.81

#define SOFTWARE_VERSION "2.00"

#define LED_YELLOW_PIN 8
#define LED_RED_PIN 12
#define LED_GREEN_PIN 10

// TODO: uppercase
#define NTP_SYNC_INTERVAL 60*60*1000
#define CHECK_NETWORK_INTERVAL 6*30*1000  // when to check for Internet connection availability
#define CHECK_CONFIG_INTERVAL 15*60*1000  // when to check for Internet connection availability
#define SEISMOMETER_TICK_INTERVAL 50
#define HTTP_RESPONSE_TIMEOUT_VALUE 5000
#define NTP_RESPONSE_TIMEOUT_VALUE 5000

#endif //GALILEO_TERREMOTI_COMMON_H
