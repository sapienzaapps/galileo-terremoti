#ifndef config_h
#define config_h


#include "GalileoLog.h"

#define DEFAULT_HTTP_SERVER "www.sapienzaapps.it"
char* path_domain = "/terremoti/galileo";
byte mac[] = { 0x00, 0x13, 0x20, 0xFF, 0x13, 0x0b };  // fictitious MAC address
typedef enum { Colossus, Panizzi, Home } DeviceLocations_t;
DeviceLocations_t deviceLocation = Home;
boolean isDhcpEnabled = false;

bool debugON = true;  // is debugging mode enabled?
bool logON = true;  // is logging mode enabled?
bool ledON = true;  // are the leds mounted on the board?
bool deviceHasSDCard = false;  // is the SD card mounted on the board?

long pingIntervalCheck = 30*1000;
long checkSensoreInterval = 60;
long NTPInterval = 15*60*1000;  // last NTP update time
long checkInternetConnectionInterval = 2*30*1000;  // when to check for Internet connection availability

bool isConnected;

double gForce = 9.81;
bool forceInitEEPROM = true;

IPAddress ip;
IPAddress dns;
IPAddress gateway;
IPAddress subnet;

// ******** FINE CONFIGURAZIONE

char* httpServer;
IPAddress timeServer;


#endif
