//
// Created by ebassetti on 21/08/15.
//

#include "../../vendor.h"
#include "AcceleroMMA7361.h"

Accelerometer* getAccelerometer() {
	return new AcceleroMMA7361();
}
