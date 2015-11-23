
#ifndef ADXL345Accelerometer_h
#define ADXL345Accelerometer_h

#include <stdint.h>
#include "../../Accelerometer.h"

#define I2C_ADDR 0x53

#define EARTH_GRAVITY_MS2   9.80665
#define SCALE_MULTIPLIER    0.004

#define DATA_FORMAT         0x31
#define BW_RATE             0x2C
#define POWER_CTL           0x2D

#define BW_RATE_1600HZ      0x0F
#define BW_RATE_800HZ       0x0E
#define BW_RATE_400HZ       0x0D
#define BW_RATE_200HZ       0x0C
#define BW_RATE_100HZ       0x0B
#define BW_RATE_50HZ        0x0A
#define BW_RATE_25HZ        0x09

#define RANGE_2G            0x00
#define RANGE_4G            0x01
#define RANGE_8G            0x02
#define RANGE_16G           0x03

#define MEASURE             0x08
#define AXES_DATA           0x32

typedef struct {
	double x;
	double y;
	double z;
} AxesInfos;

class ADXL345Accelerometer : public Accelerometer {
public:
	ADXL345Accelerometer(uint8_t i2caddress);
	double getXAccel();
	double getYAccel();
	double getZAccel();
	std::string getAccelerometerName();

	void enableMeasurement();
	void setBandwidthRate(uint8_t refreshRate);

	/**
	 * set the measurement range for 10-bit readings
	 */
	void setRange(uint8_t rangeFlags);

	/**
	 * returns the current reading from the sensor for each axis
	 *
	 * parameter gforce:
	 * * False (default): result is returned in m/s^2
	 * * True           : result is returned in gs
	 */
	AxesInfos getAxes(bool gforce);

private:
	uint8_t _i2caddress;
	int _fd;
};

#endif
