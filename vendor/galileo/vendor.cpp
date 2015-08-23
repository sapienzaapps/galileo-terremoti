//
// Created by ebassetti on 21/08/15.
//

#include "../../generic.h"
#include "AcceleroMMA7361.h"
#include "../../Log.h"
#include <Arduino.h>

Accelerometer* getAccelerometer() {
	AcceleroMMA7361* accel = new AcceleroMMA7361();

	/* Calibrating Accelerometer */
	accel->begin(A0, A1, A2);

	Log::i("Initial calibration");
	// number of samples that have to be averaged
	accel->setAveraging(10);
	accel->calibrate();

	Log::d("Calibration ended");

	return accel;
}
