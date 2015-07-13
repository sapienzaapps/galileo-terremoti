#ifndef config_h
#define config_h

#include <sstream>
#include <vector>
#include <map>
#include <string>
#include "buildcfg.h"
#include <Arduino.h>
#include <IPAddress.h>

#define DEFAULT_LOG_PATH "/media/realroot/sketch.log"
#define DEFAULT_CONFIG_PATH "/media/realroot/seismoconfig.txt"

#define GFORCE 9.81

// TODO: uppercase
#define NTPInterval 60*60*1000
#define checkInternetConnectionInterval 6*30*1000  // when to check for Internet connection availability
#define checkConfigInterval 15*60*1000  // when to check for Internet connection availability
#define HTTP_RESPONSE_TIMEOUT_VALUE 5000
#define NTP_RESPONSE_TIMEOUT_VALUE 5000
#define DHCP_CLIENT_ENABLED true

#define LED_YELLOW_PIN 8
#define LED_RED_PIN 12
#define LED_GREEN_PIN 10

typedef enum {
	Basic, Fixed
} ThresholdAlgorithm_t;

#define SOFTWARE_VERSION "1.90"

#if GALILEO_GEN == 1
#define ARDUINO_MODEL "galileo1"
#else
#define ARDUINO_MODEL "galileo2"
#endif

extern ThresholdAlgorithm_t thresholdAlgorithm;
extern bool ledON;
extern bool alert;
extern bool ForceCalibrationNeeded;
extern bool forceInitCalibration;
extern bool start;

extern bool redLedStatus;
extern bool greenLedStatus;
extern bool yellowLedStatus;

class Config {
public:
	static void init();
	static bool hasMACAddress();
	static std::string getMacAddress();
	static void setMacAddress(std::string macAddress);
	static void getMacAddressAsByte(byte mac[6]); // TODO: test
	static double getLatitude();
	static void setLatitude(double lat);
	static double getLongitude();
	static void setLongitude(double lon);
	static bool hasPosition();
	static bool isDHCPClientEnabled();
	static void getStaticNetCfg(uint32_t *staticIp, uint32_t *staticMask, uint32_t *staticGw, uint32_t *staticDns);
	static uint32_t getNTPServer();
	static bool checkServerConfig();

private:
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
