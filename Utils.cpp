//
// Created by ebassetti on 23/07/15.
//

#include <sys/sysinfo.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <cmath>
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

// workaround for Galileo
#ifdef GALILEO_GEN
unsigned long Utils::fixword(byte b1, byte b2) {
	return ((b1 << 8) | b2);
}
#else
#include <WMath.h>
unsigned long Utils::fixword(byte b1, byte b2) {
	return makeWord(b1, b2);
}
#endif
