//
// Created by Enrico on 22/11/15.
//

#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include "smbus.h"
#include <linux/i2c-dev.h>
#include <string>
#include <vector>
#include <math.h>
#include "ADXL345Accelerometer.h"

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

long ADXL345Accelerometer::getXAccel() {
	return 0;
}

long ADXL345Accelerometer::getYAccel() {
	return 0;
}

long ADXL345Accelerometer::getZAccel() {
	return 0;
}

void ADXL345Accelerometer::enableMeasurement() {
	i2c_smbus_write_byte_data(_i2caddress, POWER_CTL, MEASURE);
}

void ADXL345Accelerometer::setBandwidthRate(uint8_t refreshRate) {
	i2c_smbus_write_byte_data(_i2caddress, BW_RATE, refreshRate);
}


void ADXL345Accelerometer::setRange(uint8_t rangeFlags) {
	__s32 value = i2c_smbus_read_byte_data(_i2caddress, DATA_FORMAT);

	value &= ~0x0F;
	value |= rangeFlags;
	value |= 0x08;

	i2c_smbus_write_byte_data(_i2caddress, DATA_FORMAT, value);
}

AxesInfos ADXL345Accelerometer::getAxes(bool gforce) {
	AxesInfos ret = { 0, 0, 0 };

	i2c_smbus_write_byte_data(_i2caddress, AXES_DATA, 6);

	uint8_t bytes[6];
	i2c_smbus_read_i2c_block_data(_fd, _i2caddress, 6, bytes);

	uint16_t x = bytes[0] | (bytes[1] << 8);
	if(x & (1 << 16 - 1))
		ret.x = x - (1<<16);

	uint16_t y = bytes[2] | (bytes[3] << 8);
	if(y & (1 << 16 - 1))
		ret.y = y - (1<<16);

	uint16_t z = bytes[4] | (bytes[5] << 8);
	if(z & (1 << 16 - 1))
		ret.z = z - (1<<16);

	ret.x = ret.x * SCALE_MULTIPLIER;
	ret.y = ret.y * SCALE_MULTIPLIER;
	ret.z = ret.z * SCALE_MULTIPLIER;

	if (!gforce) {
		ret.x = ret.x * EARTH_GRAVITY_MS2;
		ret.y = ret.y * EARTH_GRAVITY_MS2;
		ret.z = ret.z * EARTH_GRAVITY_MS2;
	}

	ret.x = round(ret.x);
	ret.y = round(ret.y);
	ret.z = round(ret.z);

	return ret;
}
