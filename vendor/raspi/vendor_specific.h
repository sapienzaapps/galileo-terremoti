//
// Created by ebassetti on 23/08/15.
//

#ifndef GALILEO_TERREMOTI_VENDOR_SPECIFIC_H
#define GALILEO_TERREMOTI_VENDOR_SPECIFIC_H

#define PLATFORM_TAG         PLATFORM
#define WATCHDOG_FILE        "/tmp/galileo_watchdog"
#define DEFAULT_LOG_PATH     "/var/log/sketch.log"
#define WATCHDOG_LOG_PATH    "/var/log/watchdog.log"
#define DEFAULT_CONFIG_PATH  "/etc/seismoconfig.txt"
#define TRACEACCUMULATOR_FILE "/tmp/seismo.trc"
#define SETDATE_CMD          "/bin/date -s @%lu"
#define WATCHDOG_CRASHDIR    "/var/watchdog/"
#define STACKTRACEINFO       "/var/log/stacktrace.txt"

#define LED_YELLOW_PIN       1
#define LED_RED_PIN          2
#define LED_GREEN_PIN        0

#endif //GALILEO_TERREMOTI_VENDOR_SPECIFIC_H
