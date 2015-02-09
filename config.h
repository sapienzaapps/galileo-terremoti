#ifndef config_h
#define config_h


#include "GalileoLog.h"

#define DEFAULT_HTTP_SERVER "www.sapienzaapps.it"
char* path_domain = "/terremoti/galileo";
//byte mac[] = { 0x00, 0x13, 0x19, 0xFF, 0x14, 0x4F };  // fictitious MAC address
//byte mac[] = { 0x00, 0x13, 0x20, 0xFF, 0x14, 0x6F };  // fictitious MAC address
byte mac[] = { 0x00, 0x13, 0x20, 0xFF, 0x14, 0x9F };  // fictitious MAC address Paolo

FILE *macToFile;
char *macAddressFilePath = "media/realroot/mac_address.txt";
char* log_path = "media/realroot/log.txt";

typedef enum { Colossus, Panizzi, Home } DeviceLocations_t;
DeviceLocations_t deviceLocation = Home;
boolean isDhcpEnabled = false;

typedef enum { Basic, Fixed } ThresholdAlghoritm_t;
ThresholdAlghoritm_t thresholdAlghoritm = Basic;

bool debugON = true;  // is debugging mode enabled?
bool logON = true;  // is logging mode enabled?
bool ledON = true;  // are the leds mounted on the board?
bool deviceHasSDCard = true;  // is the SD card mounted on the board?
bool ForceCalibrationNeeded = true;// reset connection if there's not one Active
//bool resetConnection = false;// reset connection if there's not one Active
bool testNoInternet = true;// debug purpose test on local network NO Internet - Use Static IP

//long pingIntervalCheck = 30*1000; not USED
long checkSensoreInterval = 60;
long NTPInterval = 15*60*1000;  // last NTP update time
long checkInternetConnectionInterval = 2*30*1000;  // when to check for Internet connection availability

bool internetConnected;

double gForce = 9.81;  // gravity force
bool forceInitEEPROM = false;


#endif
