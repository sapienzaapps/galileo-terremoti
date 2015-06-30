#ifndef config_h
#define config_h

#include <Arduino.h>

#define DEFAULT_HTTP_SERVER "www.sapienzaapps.it"
#define DEFAULT_HTTP_PATH "/terremoti/galileo"
#define DEFAULT_LOG_PATH "/media/realroot/sketch.log"
#define DEFAULT_ACC_PATH "/media/realroot/acc.txt"
#define DEFAULT_CONFIG_PATH "/media/realroot/seismoconfig.txt"

#define GFORCE 9.81

// TODO: uppercase
#define NTPInterval 60*60*1000
#define checkInternetConnectionInterval 6*30*1000  // when to check for Internet connection availability
#define checkConfigInterval 15*60*1000  // when to check for Internet connection availability
#define timeoutResponse 5000
#define isDhcpEnabled true

#define LED_YELLOW 8
#define LED_RED 12
#define LED_GREEN 10

typedef enum {
	Basic, Fixed
} ThresholdAlgorithm_t;

typedef struct _configFile {
	char lat[10] = "00.000000";
	char lon[10] = "00.000000";
#if GEN2 > 0
  char *model = "galileo2";
  float version = 1.90;
#else
	char *model = "galileo1";
	float version = 1.90;
#endif
} ConfigFile;


extern ThresholdAlgorithm_t thresholdAlgorithm;
extern char mac_string[18];
extern byte mac[6];
extern bool ledON;
extern bool alert;
extern bool ForceCalibrationNeeded;
extern bool testNoInternet;
extern bool request_mac_from_server;
extern bool request_lat_lon;
extern bool forceInitEEPROM;
extern bool internetConnected;
extern bool start;

extern bool redLedStatus;
extern bool greenLedStatus;
extern bool yellowLedStatus;

extern ConfigFile configGal;

#endif 
