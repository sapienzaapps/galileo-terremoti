#ifndef config_h
#define config_h


#include "GalileoLog.h"

#define DEFAULT_HTTP_SERVER "www.sapienzaapps.it"
#define SUCCESS 1 
char* path_domain = "/terremoti/galileo";
//byte mac[] = { 0x00, 0x13, 0x19, 0xFF, 0x14, 0x4F };  // fictitious MAC address
//byte mac[] = { 0x00, 0x13, 0x20, 0xFF, 0x14, 0x6F };  // fictitious MAC address DANIELE
//byte mac[] = { 0x00, 0x13, 0x29, 0xFF, 0x14, 0x6F };  // fictitious MAC address DANIELE
byte mac[] = { 0x00, 0x13, 0x20, 0xFF, 0x15, 0x9F };  // fictitious MAC address Gen2 Panizzi
//byte mac[] = { 0x00, 0x13, 0x20, 0xFF, 0x17, 0x9F };  // fictitious MAC address Gen1 Panizzi
// byte mac[] = { 0x00, 0x13, 0x20, 0xFF, 0x17, 0x9E };  // fictitious MAC address Gen1 BEATRICE
// char *mac_string_default = "001320ff146f"; // default Daniele gen1
char *mac_string_default = "001320ff147f"; // default Daniele gen2
char mac_string[18]; // default Daniele
FILE *macToFile;
char *macAddressFilePath = "media/realroot/mac_address.txt";
char* log_path = "media/realroot/log.txt";
char* config_path = "media/realroot/seismoconfig.txt";

/* struct configFile {
  byte mac[6];
  float lat;
  float lon;
}configuration; */


typedef enum { Colossus, Panizzi, Home } DeviceLocations_t;
DeviceLocations_t deviceLocation = Home;
boolean isDhcpEnabled = true;

typedef enum { Basic, Fixed } ThresholdAlghoritm_t;
ThresholdAlghoritm_t thresholdAlghoritm = Basic;

bool debugON = true;  // is debugging mode enabled?
bool logON = false;  // is logging mode enabled?
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

unsigned long checkSensoreInterval = 60;
unsigned long NTPInterval = 60*60*1000;  // last NTP update time
unsigned long checkInternetConnectionInterval = 6*30*1000;  // when to check for Internet connection availability
unsigned long timeoutResponse = 5000;

double gForce = 9.81;  // gravity force
struct configFile{
  float lat = 0.000000;
  float lon = 0.000000;
  #if GEN2 > 0
  char *model = "galileo2";
#else
  char *model = "galileo";
#endif
  float version = 1.4;
} configGal;

void printConfig(){
  Serial.println("######## Config ########## ");
  Serial.print("Lat: ");
  Serial.print(configGal.lat);
  Serial.print("\tLong: ");
  Serial.println(configGal.lon );
  Serial.print("model: ");
  Serial.println(configGal.model );
  Serial.print("version: ");
  Serial.println(configGal.version );
  Serial.print("errors: ");
  Serial.println(errors_connection);
  Serial.println("######## Config end ########## ");
   
}
/* float lat = 41.283799;
float lon = 13.251675;
// float version = 1.3;
float version = 1.4;
#if GEN2 > 0
  char *model = "galileo2";
#else
  char *model = "galileo";
#endif
*/
#endif 
