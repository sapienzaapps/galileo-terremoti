//
// Created by ebassetti on 21/08/15.
//

#ifndef GALILEO_TERREMOTI_VENDOR_H
#define GALILEO_TERREMOTI_VENDOR_H

#include "Accelerometer.h"

void vendor_init(int argc, char** argv);
Accelerometer* getAccelerometer();
std::string getPlatformName();

#endif //GALILEO_TERREMOTI_VENDOR_H
