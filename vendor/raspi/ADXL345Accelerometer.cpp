//
// Created by Enrico on 22/11/15.
//

#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include "smbus.h"
#include <string>
#include <vector>
#include <math.h>
#include "ADXL345Accelerometer.h"

#define I2C_SLAVE       0x0703  /* Use this slave address */

ADXL345Accelerometer::ADXL345Accelerometer(uint8_t i2caddress) {
	_i2caddress = i2caddress;

	_fd = open("/dev/i2c-1", O_RDWR);
	ioctl(_fd, I2C_SLAVE, I2C_ADDR);

	setBandwidthRate(BW_RATE_100HZ);
	setRange(RANGE_2G);
	enableMeasurement();
}

std::string ADXL345Accelerometer::getAccelerometerName() {
	return std::string("ADXL345");
}

double ADXL345Accelerometer::getXAccel() {
	AxesInfos ax = getAxes(false);
	return ax.x;
}

double ADXL345Accelerometer::getYAccel() {
	AxesInfos ax = getAxes(false);
	return ax.y;
}

double ADXL345Accelerometer::getZAccel() {
	AxesInfos ax = getAxes(false);
	return ax.z;
}

void ADXL345Accelerometer::enableMeasurement() {
	i2c_smbus_write_byte_data(_fd, POWER_CTL, MEASURE);
}

void ADXL345Accelerometer::setBandwidthRate(uint8_t refreshRate) {
	i2c_smbus_write_byte_data(_fd, BW_RATE, refreshRate);
}


void ADXL345Accelerometer::setRange(uint8_t rangeFlags) {
	__s32 value = i2c_smbus_read_byte_data(_fd, DATA_FORMAT);

	value &= ~0x0F;
	value |= rangeFlags;
	value |= 0x08;

	i2c_smbus_write_byte_data(_fd, DATA_FORMAT, value);
}

AxesInfos ADXL345Accelerometer::getAxes(bool gforce) {
	AxesInfos ret = { 0, 0, 0 };

	uint8_t bytes[6];
	i2c_smbus_read_i2c_block_data(_fd, AXES_DATA, 6, bytes);

	ret.x = bytes[0] | ((int8_t)bytes[1] << 8);
	ret.y = bytes[2] | ((int8_t)bytes[3] << 8);
	ret.z = bytes[4] | ((int8_t)bytes[5] << 8);

	ret.x = ret.x * SCALE_MULTIPLIER;
	ret.y = ret.y * SCALE_MULTIPLIER;
	ret.z = ret.z * SCALE_MULTIPLIER;

	if (!gforce) {
		ret.x = ret.x * EARTH_GRAVITY_MS2;
		ret.y = ret.y * EARTH_GRAVITY_MS2;
		ret.z = ret.z * EARTH_GRAVITY_MS2;
	}

	return ret;
}
