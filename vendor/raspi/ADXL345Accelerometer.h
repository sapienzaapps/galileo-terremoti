
#ifndef ADXL345Accelerometer_h
#define ADXL345Accelerometer_h

#include <stdint.h>
#include "../../Accelerometer.h"

class ADXL345Accelerometer : public Accelerometer {
public:
	ADXL345Accelerometer(uint8_t i2caddress);
	long getXAccel();
	long getYAccel();
	long getZAccel();
	std::string getAccelerometerName();

private:
	uin8_t i2caddress;
};

#endif
