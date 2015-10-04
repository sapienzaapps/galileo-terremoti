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
#define DEFAULT_LOG_PATH     "/media/realroot/sketch.log"
#define WATCHDOG_LOG_PATH    "/media/realroot/watchdog.log"
#define DEFAULT_CONFIG_PATH  "/media/realroot/seismoconfig.txt"
#define CALIBRATION_FILE     "/media/realroot/calibration.dat"
#define REBOOT_CMD           "reboot"
#define SETDATE_CMD          "/bin/date -s @%lu"
#define WATCHDOG_CRASHDIR    "/media/realroot/watchdog/"

#endif //GALILEO_TERREMOTI_VENDOR_SPECIFIC_H
