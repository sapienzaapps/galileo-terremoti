//
// Created by ebassetti on 23/07/15.
//

#ifndef GALILEO_TERREMOTI_UTILS_H
#define GALILEO_TERREMOTI_UTILS_H

#include <string>
#include <sys/stat.h>
#include "common.h"

/**
 * Utils class
 */
class Utils {
public:
	/**
	 * Get free RAM
	 * @return Free RAM if available, zero if value is not available
	 */
	static unsigned long getFreeRam();

	/**
	 * Check if file exists
	 * @param filename File name/path
	 * @return True if file exists, false otherwise
	 */
	static bool fileExists(const char *filename);

	/**
	 * Get file size
	 * @param filename File name/path
	 * @return File size
	 */
	static ssize_t fileSize(const char *filename);

	/**
	 * Ascii to float/double
	 * @param buf String buffer
	 * @param max String size
	 * @return Double value
	 */
	static double atofn(const char *buf, size_t max);

	/**
	 * Absolute average
	 * @param buf Array of float
	 * @param size Size
	 * @param Average
	 */
	static float absavg(float *buf, int size);

	/**
	 * Standard dev
	 * @param buf Array of integers
	 * @param size Size
	 * @param avg Average
	 * @param Standard dev
	 */
	static double stddev(float *buf, int size, float avg);

	/**
	 * Delay for a specified amount of milliseconds
	 * @param ms Milliseconds
	 */
	static void delay(unsigned int ms);

	/**
	 * Get current milliseconds from sketch boot
	 * @return Milliseconds
	 */
	static uint32_t millis();

	/**
	 * 64-bit endianess converter
	 * @param bignum 64-bit byffer
	 * @return 64-bit integer converted
	 */
	static uint64_t hton64(byte* bignum);

	/**
	 * Reverse a float value
	 * @param inFloat Original float
	 * @return Reversed float
	 */
	static float reverseFloat(const float inFloat);

	/**
	 * Trim string
	 * @param str String to trim
	 * @param c Char to trim
	 * @return Trimmed string
	 */
	static std::string trim(std::string& str, char c);

	/**
	 * Get interface MAC address
	 * @return MAC Address
	 */
	static std::string getInterfaceMAC();

	/**
	 * Get interface MAC address and name
	 * @param intfname Buffer where place interface name
	 * @param intfnamesize Buffer size
	 * @return MAC Address
	 */
	static std::string getInterfaceMAC(char* intfname, size_t intfnamesize);

	/**
	 * Get device uptime
	 * @return Uptime in seconds
	 */
	static uint32_t uptime();

	/**
	 * Read the first line of file
	 * @param filename File
	 * @return The first name of file, empty string if error
	 */
	static std::string readFirstLine(std::string filename);

	/**
	 * Set file-descriptor to a non-blocking I/0
	 * @param fd File descriptor
	 * @return -1 if error, other values if no error
	 */
	static int setNonblocking(int fd);

	/**
	 * Generate a string from native value
	 * @param d Value to convert to string
	 * @param String value
	 */
	static std::string toString(double d);

	/**
	 * Generate a string from native value
	 * @param d Value to convert to string
	 * @param String value
	 */
	static std::string toString(long d);

	/**
	 * Generate a string from native value
	 * @param d Value to convert to string
	 * @param String value
	 */
	static std::string toString(unsigned long d);

	/**
	 * Generate a string from native value
	 * @param d Value to convert to string
	 * @param String value
	 */
	static std::string Utils::toString(uint64_t d);

	/**
	 * Generate a string from native value
	 * @param d Value to convert to string
	 * @param String value
	 */
	static std::string toString(uint32_t d);

	/**
	 * Generate a string from native value
	 * @param d Value to convert to string
	 * @param String value
	 */
	static std::string toString(int d);
};

#endif //GALILEO_TERREMOTI_UTILS_H
