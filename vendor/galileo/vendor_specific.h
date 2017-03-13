//
// Created by ebassetti on 23/08/15.
//

#ifndef GALILEO_TERREMOTI_VENDOR_SPECIFIC_H
#define GALILEO_TERREMOTI_VENDOR_SPECIFIC_H

#if GALILEO_GEN == 1
#define PLATFORM_TAG         "galileo1"
#else
#define PLATFORM_TAG         "galileo2"
#endif

#define WATCHDOG_FILE        "/tmp/galileo_watchdog"
#define TRACEACCUMULATOR_FILE "/media/realroot/seismo.trc"
#define DEFAULT_LOG_PATH     "/media/realroot/sketch.log"
#define WATCHDOG_LOG_PATH    "/media/realroot/watchdog.log"
#define DEFAULT_CONFIG_PATH  "/media/realroot/seismoconfig.txt"
#define WATCHDOG_CRASHDIR    "/media/realroot/watchdog/"
#define STACKTRACEINFO       "/media/realroot/stacktrace.txt"

#define LED_YELLOW_PIN       8
#define LED_RED_PIN          12
#define LED_GREEN_PIN        10

#endif //GALILEO_TERREMOTI_VENDOR_SPECIFIC_H
