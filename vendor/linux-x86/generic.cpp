//
// Created by ebassetti on 21/08/15.
//

#include "../../generic.h"
#include "DummyAccelerometer.h"

Accelerometer* getAccelerometer() {
	return new DummyAccelerometer();
}

std::string getPlatformName() {
	return "Linux Generic";
}
