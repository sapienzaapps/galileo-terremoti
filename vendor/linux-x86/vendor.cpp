//
// Created by ebassetti on 21/08/15.
//

#include "../../vendor.h"
#include "DummyAccelerometer.h"

Accelerometer* getAccelerometer() {
	return new DummyAccelerometer();
}
