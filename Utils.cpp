//
// Created by ebassetti on 23/07/15.
//

#include <sys/sysinfo.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <cmath>
#include <sstream>
#include "common.h"
#include "Utils.h"

unsigned long Utils::freeRam() {
	struct sysinfo sys_info;
	if (sysinfo(&sys_info) == 0) {
		return sys_info.freeram;
	} else {
		return 0;
	}
}

bool Utils::fileExists(const char *filename) {
	return access(filename, F_OK) != -1;
}

double Utils::atofn(char *buf, size_t max)  {
	size_t bufLen = strlen(buf);
	max = (max <= bufLen ? max : bufLen);
	char termination = buf[max];
	buf[max] = 0;
	double ret = atof(buf);
	buf[max] = termination;
	return ret;
}

float Utils::absavg(int *buf, int size) {
	float ret = 0;
	for (int i = 0; i < size; i++) {
		ret += (buf[i] < 0 ? buf[i] * -1 : buf[i]);
	}
	return ret / size;
}

// Standard Deviation
double Utils::stddev(int *buf, int size, float avg) {
	// Formula: RAD ( SUM{i,size}( (x[i] - avg)^2 ) / (size - 1) )
	double sum = 0;
	for (int i = 0; i < size; i++) {
		sum += pow(buf[i] - avg, 2);
	}

	return sqrt(sum / (size - 1));
}

void Utils::delay(unsigned int ms) {
	usleep(ms * 1000);
}

uint32_t Utils::millis() {
	struct timespec ts;
	unsigned theTick = 0U;
	clock_gettime( CLOCK_REALTIME, &ts );
	theTick  = ts.tv_nsec / 1000000;
	theTick += ts.tv_sec * 1000;
	return theTick;
}

uint64_t Utils::hton64(byte* bignum) {
	uint64_t aux = 0;
	uint8_t *p = (uint8_t*)bignum;
	int i;

	/* we get the ntp in network byte order, so we must
	 * convert it to host byte order. */
	for (i = 0; i < 4; i++) {
		aux <<= 8;
		aux |= *p++;
	} /* for */
	return aux;
}

float Utils::reverseFloat(const float inFloat) {
	float retVal;
	char *floatToConvert = ( char* ) & inFloat;
	char *returnFloat = ( char* ) & retVal;

	// swap the bytes into a temporary buffer
	returnFloat[0] = floatToConvert[3];
	returnFloat[1] = floatToConvert[2];
	returnFloat[2] = floatToConvert[1];
	returnFloat[3] = floatToConvert[0];

	return retVal;
}


#ifndef __IS_GALILEO
void delay(unsigned int ms) {
	Utils::delay(ms);
}

uint32_t millis() {
	return Utils::millis();
}
#endif

std::string Utils::doubleToString(double d) {
	std::ostringstream strs;
	strs << d;
	return strs.str();
}
