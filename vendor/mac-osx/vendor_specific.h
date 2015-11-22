//
// Created by ebassetti on 23/08/15.
//

#ifndef GALILEO_TERREMOTI_VENDOR_SPECIFIC_H
#define GALILEO_TERREMOTI_VENDOR_SPECIFIC_H

#define PLATFORM_TAG         PLATFORM
#define WATCHDOG_FILE        "/tmp/galileo_watchdog"
#define DEFAULT_LOG_PATH     "/tmp/sketch.log"
#define WATCHDOG_LOG_PATH    "/tmp/watchdog.log"
#define DEFAULT_CONFIG_PATH  "/tmp/seismoconfig.txt"
#define CALIBRATION_FILE     "/tmp/calibration.dat"
#define SETDATE_CMD          "/bin/date -f \"%%s\" %lu"
#define WATCHDOG_CRASHDIR    "/tmp/watchdog/"
#define STACKTRACEINFO       "/tmp/stacktrace.txt"

#define LED_YELLOW_PIN       0
#define LED_RED_PIN          0
#define LED_GREEN_PIN        0

#endif //GALILEO_TERREMOTI_VENDOR_SPECIFIC_H
