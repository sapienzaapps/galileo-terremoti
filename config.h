#ifndef config_h
#define config_h


#include "GalileoLog.h"

#define DEFAULT_HTTP_SERVER "www.sapienzaapps.it"
char* path_domain = "/terremoti/galileo";
byte mac[] = { 0x00, 0x13, 0x19, 0xFF, 0x14, 0x4F };  // fictitious MAC address

FILE *macToFile;
char *macAddressFilePath = "media/realroot/mac_address.txt";

typedef enum { Colossus, Panizzi, Home } DeviceLocations_t;
DeviceLocations_t deviceLocation = Home;
boolean isDhcpEnabled = false;

typedef enum { Basic, Fixed } ThresholdAlghoritm_t;
ThresholdAlghoritm_t thresholdAlghoritm = Basic;

char* log_path = "media/realroot/log.txt";

bool debugON = false;  // is debugging mode enabled?
bool logON = true;  // is logging mode enabled?
bool ledON = false;  // are the leds mounted on the board?
bool deviceHasSDCard = false;  // is the SD card mounted on the board?

long pingIntervalCheck = 30*1000;
long checkSensoreInterval = 60;
long NTPInterval = 15*60*1000;  // last NTP update time
long checkInternetConnectionInterval = 2*30*1000;  // when to check for Internet connection availability

bool internetConnected;

double gForce = 9.81;  // gravity force
bool forceInitEEPROM = false;


#endif
