//
// Created by ebassetti on 23/07/15.
//

#ifndef GALILEO_TERREMOTI_UTILS_H
#define GALILEO_TERREMOTI_UTILS_H

#include <string>
#include <sys/stat.h>
#include "common.h"

class Utils {
public:
	static unsigned long getFreeRam();
	static bool fileExists(const char *filename);
	static size_t fileSize(const char *filename);
	static double atofn(const char *buf, size_t max);
	static float absavg(int *buf, int size);
	static double stddev(int *buf, int size, float avg);
	static void delay(unsigned int ms);
	static uint32_t millis();
	static uint64_t hton64(byte* bignum);
	static float reverseFloat(const float inFloat);
	static std::string trim(std::string& str, char c);
	static std::string getInterfaceMAC();
	static uint32_t uptime();
	static std::string readFirstLine(std::string filename);
	static int setNonblocking(int);
	static std::string toString(double);
	static std::string toString(long d);
	static std::string toString(unsigned long d);
	static std::string toString(uint32_t d);
	static std::string toString(int d);
};

#endif //GALILEO_TERREMOTI_UTILS_H
