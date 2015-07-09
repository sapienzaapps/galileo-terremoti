#include <string>
#include "config.h"
#include "commons.h"

ThresholdAlgorithm_t thresholdAlgorithm = Basic;

bool ledON = true;  // are the leds mounted on the board?
bool alert = true;  // select communication type for Events
bool ForceCalibrationNeeded = true;

// reset connection if there's not one Active
bool start = false;
bool forceInitEEPROM = false;

bool redLedStatus = false;
bool greenLedStatus = false;
bool yellowLedStatus = false;

std::string Config::macAddress = "";
double Config::lat = 0.0;
double Config::lon = 0.0;

bool Config::hasMACAddress() {
	return !Config::macAddress.empty();
}

bool Config::hasPosition() {
	return Config::lat != 0.0 && Config::lon != 0.0;
};

std::string Config::getMacAddress() {
	return Config::macAddress;
}

void Config::setMacAddress(std::string macAddress) {
	Config::macAddress = macAddress;
}

void Config::getMacAddressAsByte(byte mac[6]) {
	for (int i = 0; i < 6; i++) {
		char b[3] = {0};
		b[0] = Config::macAddress[i * 2];
		b[1] = Config::macAddress[(i * 2) + 1];
		mac[i] = (byte) strtol(b, NULL, 16);
	}
}

double Config::getLatitude() {
	return Config::lat;
}

void Config::setLatitude(double lat) {
	Config::lat = lat;
}

double Config::getLongitude() {
	return Config::lon;
}

void Config::setLongitude(double lon) {
	Config::lon = lon;
}

void Config::readConfigFile(const char *filepath) {
	Log::i("Reading config file %s", filepath);
	FILE *fp = fopen(filepath, "r");
	if (fp == NULL) {
		Log::e("Error opening config file");
		return;
	}

	char buf[100 + 1];
	char *argument;
	while (fgets(buf, 100, fp) != NULL) {
		argument = strchr(buf, ':');
		if (argument == NULL) continue;
		argument++;
		if (strncmp("deviceid", buf, 8) == 0) {
			if (strlen(argument) < 12) continue;
			Config::macAddress = std::string(argument);
			Log::d("Device ID: %s", Config::getMacAddress().c_str());
		} else if (strncmp("lat", buf, 3) == 0) {
			Config::lat = atofn(argument, 8);
			Log::d("Latitude: %lf", Config::getLatitude());
		} else if (strncmp("lon", buf, 3) == 0) {
			Config::lon = atofn(argument, 8);
			Log::d("Longitude: %lf", Config::getLongitude());
		}
	}
	Log::i("Config file read OK");
}
