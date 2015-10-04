#ifndef config_h
#define config_h

#include <sstream>
#include <vector>
#include <map>
#include "common.h"
#include "Seismometer.h"

class Config {
public:
	static void init();
	static bool hasMACAddress();
	static std::string getMacAddress();
	static void getMacAddressAsByte(byte mac[6]); // TODO: test
	static double getLatitude();
	static double getLongitude();
	static bool hasPosition();
	static bool checkServerConfig();
	static void printConfig();
	static void setMacAddress(std::string macAddress);
	static void setLatitude(double lat);
	static void setLongitude(double lon);
	static uint32_t getSyslogServer();
	static void save();

private:
	static void loadDefault();
	static bool readConfigFile(const char *filepath);
	static std::map<std::string, std::string> &configSplit(const std::string &s, char delim, std::map<std::string, std::string> &elems);
	static std::map<std::string, std::string> configSplit(const std::string &s, char delim);
	static void file_put_contents(const char* path, std::string content);

	static double lat;
	static double lon;
	static std::string macAddress;
	static uint32_t syslogServer;
};

#endif 
