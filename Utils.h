//
// Created by ebassetti on 23/07/15.
//

#ifndef GALILEO_TERREMOTI_UTILS_H
#define GALILEO_TERREMOTI_UTILS_H

#include <BitsAndBytes.h>

class Utils {
public:
	static unsigned long freeRam();
	static bool fileExists(const char *filename);
	static double atofn(char *buf, size_t max);
	static float absavg(int *buf, int size);
	static double stddev(int *buf, int size, float avg);
	static unsigned long fixword(byte b1, byte b2);
};

#endif //GALILEO_TERREMOTI_UTILS_H
