#ifndef config_h
#define config_h

#include <string>
#include "buildcfg.h"
#include <Arduino.h>

#define DEFAULT_LOG_PATH "/media/realroot/sketch.log"
#define DEFAULT_ACC_PATH "/media/realroot/acc.txt"
#define DEFAULT_CONFIG_PATH "/media/realroot/seismoconfig.txt"

#define GFORCE 9.81

// TODO: uppercase
#define NTPInterval 60*60*1000
#define checkInternetConnectionInterval 6*30*1000  // when to check for Internet connection availability
#define checkConfigInterval 15*60*1000  // when to check for Internet connection availability
#define HTTP_RESPONSE_TIMEOUT_VALUE 5000
#define NTP_RESPONSE_TIMEOUT_VALUE 5000
#define DHCP_CLIENT_ENABLED true

#define LED_YELLOW 8
#define LED_RED 12
#define LED_GREEN 10

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
extern bool forceInitEEPROM;
extern bool start;

extern bool redLedStatus;
extern bool greenLedStatus;
extern bool yellowLedStatus;

class Config {
public:
	static void readConfigFile(const char *filepath);
	static bool hasMACAddress();
	static std::string getMacAddress();
	static void setMacAddress(std::string macAddress);
	static void getMacAddressAsByte(byte mac[6]); // TODO: test
	static double getLatitude();
	static void setLatitude(double lat);
	static double getLongitude();
	static void setLongitude(double lon);
	static bool hasPosition();
private:
	static double lat;
	static double lon;
	static std::string macAddress;
};

#endif 
