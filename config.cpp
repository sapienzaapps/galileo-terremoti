
#include "config.h"

ThresholdAlgorithm_t thresholdAlgorithm = Basic;

char mac_string[18];

byte mac[6] = { 0x00, 0x13, 0x20, 0xFF, 0x15, 0x9F };
bool ledON = true;  // are the leds mounted on the board?
bool alert = true;  // select communication type for Events
bool ForceCalibrationNeeded = true;// reset connection if there's not one Active
bool testNoInternet = true;// debug purpose test on local network NO Internet - Use Static IP
bool request_mac_from_server = true;
bool request_lat_lon = true;
bool forceInitEEPROM = false;
bool internetConnected = false;
bool start = false;

ConfigFile config;

bool redLedStatus = false;
bool greenLedStatus = false;
bool yellowLedStatus = false;