//
// Created by ebassetti on 21/08/15.
//

#include "DummyAccelerometer.h"

double DummyAccelerometer::getXAccel() {
	return 0;
}

double DummyAccelerometer::getYAccel() {
	return 0;
}

double DummyAccelerometer::getZAccel() {
	return 0;
}

DummyAccelerometer::DummyAccelerometer() {
}

std::string DummyAccelerometer::getAccelerometerName() {
	return std::string("Dummy");
}
