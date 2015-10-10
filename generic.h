//
// Created by ebassetti on 21/08/15.
//

#ifndef GALILEO_TERREMOTI_VENDOR_H
#define GALILEO_TERREMOTI_VENDOR_H

#include "Accelerometer.h"

/**
 * Vendor init function
 * @param argc Arguments number
 * @param argv Arguments
 */
void vendor_init(int argc, char** argv);

/**
 * Return accelerometer object
 * @return Accelerometer object
 */
Accelerometer* getAccelerometer();

/**
 * Get platform name (user-friendly)
 * @return Platform-name
 */
std::string getPlatformName();

/**
 * Reboot device
 */
void platformReboot();

#endif //GALILEO_TERREMOTI_VENDOR_H
