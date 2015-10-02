//
// Created by ebassetti on 21/08/15.
//

#include "../../generic.h"
#include "DummyAccelerometer.h"


void vendor_init(int argc, char** argv) {
}

Accelerometer* getAccelerometer() {
	return new DummyAccelerometer();
}

std::string getPlatformName() {
	return "OSX Generic";
}
