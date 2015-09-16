//
// Created by ebassetti on 23/07/15.
//

#ifndef GALILEO_TERREMOTI_UTILS_H
#define GALILEO_TERREMOTI_UTILS_H

#include <string>
#include "common.h"

class Utils {
public:
	static unsigned long freeRam();
	static bool fileExists(const char *filename);
	static double atofn(const char *buf, size_t max);
	static float absavg(int *buf, int size);
	static double stddev(int *buf, int size, float avg);
	static void delay(unsigned int ms);
	static uint32_t millis();
	static uint64_t hton64(byte* bignum);
	static std::string doubleToString(double);
	static float reverseFloat(const float inFloat);
	static std::string trim(std::string& str, char c);
	static std::string getInterfaceMAC();
	static uint32_t uptime();
	static std::string readFirstLine(std::string filename);
};

#endif //GALILEO_TERREMOTI_UTILS_H
