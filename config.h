#ifndef config_h
#define config_h


#include "GalileoLog.h"

#define DEFAULT_HTTP_SERVER "www.sapienzaapps.it"
char* path_domain = "/terremoti/galileo";
//byte mac[] = { 0x00, 0x13, 0x19, 0xFF, 0x14, 0x4F };  // fictitious MAC address
byte mac[] = { 0x00, 0x13, 0x20, 0xFF, 0x14, 0x6F };  // fictitious MAC address DANIELE
//byte mac[] = { 0x00, 0x13, 0x20, 0xFF, 0x15, 0x9F };  // fictitious MAC address Gen2 Panizzi
//byte mac[] = { 0x00, 0x13, 0x20, 0xFF, 0x17, 0x9F };  // fictitious MAC address Gen1 Panizzi
// byte mac[] = { 0x00, 0x13, 0x20, 0xFF, 0x17, 0x9E };  // fictitious MAC address Gen1 BEATRICE
char *mac_string_default = "001320ff146f"; // default Daniele
char mac_string[18]; // default Daniele
FILE *macToFile;
char *macAddressFilePath = "media/realroot/mac_address.txt";
char* log_path = "media/realroot/log.txt";
char* config_path = "media/realroot/seismoconfig.txt";

struct configFile {
  byte mac[6];
  float lat;
  float lon;
}configuration;


typedef enum { Colossus, Panizzi, Home } DeviceLocations_t;
DeviceLocations_t deviceLocation = Home;
boolean isDhcpEnabled = false;

typedef enum { Basic, Fixed } ThresholdAlghoritm_t;
ThresholdAlghoritm_t thresholdAlghoritm = Basic;

bool debugON = true;  // is debugging mode enabled?
bool logON = true;  // is logging mode enabled?
bool ledON = true;  // are the leds mounted on the board?
bool alert = true;  // select communication type for Events
bool deviceHasSDCard = true;  // is the SD card mounted on the board?
bool ForceCalibrationNeeded = true;// reset connection if there's not one Active
bool testNoInternet = true;// debug purpose test on local network NO Internet - Use Static IP
//bool resetConnection = false;// reset connection if there's not one Active
bool request_mac_from_server = true;
bool request_lat_lon = true;
bool forceInitEEPROM = false;
bool internetConnected = false;
bool start = false;

//long pingIntervalCheck = 30*1000; not USED
unsigned long checkSensoreInterval = 60;
unsigned long NTPInterval = 15*60*1000;  // last NTP update time
unsigned long checkInternetConnectionInterval = 10*30*1000;  // when to check for Internet connection availability
unsigned long timeoutResponse = 5000;

double gForce = 9.81;  // gravity force
float lat = 99.10;
float lon = 35.72;
float version = 1.3;
char *model = "galileo";


#endif
