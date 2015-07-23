#ifndef config_h
#define config_h

#include <sstream>
#include <vector>
#include <map>
#include <string>
#include <BitsAndBytes.h>
#include "common.h"
#include "Seismometer.h"

// TODO: uppercase
#define NTPInterval 60*60*1000
#define checkInternetConnectionInterval 6*30*1000  // when to check for Internet connection availability
#define checkConfigInterval 15*60*1000  // when to check for Internet connection availability
#define HTTP_RESPONSE_TIMEOUT_VALUE 5000
#define NTP_RESPONSE_TIMEOUT_VALUE 5000

class Config {
public:
	static void init();
	static bool hasMACAddress();
	static std::string getMacAddress();
	static void getMacAddressAsByte(byte mac[6]); // TODO: test
	static double getLatitude();
	static double getLongitude();
	static bool hasPosition();
	static bool isDHCPClientEnabled();
	static void getStaticNetCfg(uint32_t *staticIp, uint32_t *staticMask, uint32_t *staticGw, uint32_t *staticDns);
	static uint32_t getNTPServer();
	static bool checkServerConfig();
	static void printConfig();
	static void setMacAddress(std::string macAddress);
	static void setLatitude(double lat);
	static void setLongitude(double lon);

private:
	static void save();
	static void loadDefault();
	static bool readConfigFile(const char *filepath);
	static std::map<std::string, std::string> &configSplit(const std::string &s, char delim, std::map<std::string, std::string> &elems);
	static std::map<std::string, std::string> configSplit(const std::string &s, char delim);
	static void file_put_contents(const char* path, std::string content);

	static double lat;
	static double lon;
	static std::string macAddress;
	static bool dhcpClientEnabled;
	static uint32_t staticIp;
	static uint32_t staticMask;
	static uint32_t staticGw;
	static uint32_t staticDns;
	static uint32_t ntpServer;
};

#endif 
