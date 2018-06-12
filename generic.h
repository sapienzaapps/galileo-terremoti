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

void platformUpgrade(std::string path);

void execSystemTimeUpdate(time_t epoch);

unsigned long getUNIXTime();

#endif //GALILEO_TERREMOTI_VENDOR_H
