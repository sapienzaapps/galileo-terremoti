#include <string>
#include "Config.h"
#include "commons.h"
#include "HTTPClient.h"
#include "NTP.h"

ThresholdAlgorithm_t thresholdAlgorithm = Basic;

bool ledON = true;  // are the leds mounted on the board?
bool alert = true;  // select communication type for Events
bool ForceCalibrationNeeded = true;

// reset connection if there's not one Active
bool start = false;
bool forceInitCalibration = false;

bool redLedStatus = false;
bool greenLedStatus = false;
bool yellowLedStatus = false;

std::string Config::macAddress = "";
double Config::lat = 0.0;
double Config::lon = 0.0;
bool Config::dhcpClientEnabled = true;
uint32_t Config::staticIp = 0;
uint32_t Config::staticMask = 0;
uint32_t Config::staticGw = 0;
uint32_t Config::staticDns = 0;
uint32_t Config::ntpServer = 0;

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
		char b[3] = {0, 0, 0};
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

bool Config::readConfigFile(const char *filepath) {
	Log::i("Reading config file %s", filepath);
	FILE *fp = fopen(filepath, "r");
	if (fp == NULL) {
		Log::e("Error opening config file");
		return false;
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
	return true;
}

void Config::init() {
	bool cfgOk = readConfigFile(DEFAULT_CONFIG_PATH);
	if(!cfgOk) {
		loadDefault();
	}
}

void Config::loadDefault() {
	lat = 0.0;
	lon = 0.0;
	macAddress = "";
	dhcpClientEnabled = true;
	staticIp = 0;
	staticMask = 0;
	staticGw = 0;
	staticDns = 0;
	ntpServer = 0;
}

bool Config::isDHCPClientEnabled() {
	return Config::dhcpClientEnabled;
}

uint32_t Config::getNTPServer() {
	return Config::ntpServer;
}

void Config::getStaticNetCfg(uint32_t *ostaticIp, uint32_t *ostaticMask, uint32_t *ostaticGw, uint32_t *ostaticDns) {
	*ostaticIp = staticIp;
	*ostaticMask = staticMask;
	*ostaticGw = staticGw;
	*ostaticDns = staticDns;
}

bool Config::checkServerConfig() {
	std::string cfg = HTTPClient::getConfig();
	if(!cfg.empty()) {
		std::map<std::string, std::string> params = configSplit(cfg, '|');

		std::string path = params["path"];
		if(!path.empty()) {
			// Firmware update
			char cmd[1024];
			memset(cmd, 0, 1024);
			snprintf(cmd, 1023, "curl -o /media/realroot/sketch.new %s");
			system(cmd);

			// TODO: update!
			system("reboot");
			exit(0);
		}

		HTTPClient::setBaseURL(params["server"]);

		IPAddress ntpserver((const uint8_t *)params["ntpserver"].c_str());
		NTP::setNTPServer(ntpserver);

		std::string script = params["script"];
		if (!script.empty()) {
			Log::d("Script Creation (len: %i path: %s...", script.size(), "/tmp/script.sh");
			file_put_contents("/tmp/script.sh", script);
			Log::d("Script created, executing...");
			system("chmod +x /tmp/script.sh");
			system("/tmp/script.sh");
		}

		// TODO: if network settings changed, save config and reboot

		return true;
	} else {
		return false;
	}
}

std::map<std::string, std::string> &Config::configSplit(const std::string &s, char delim,
												std::map<std::string, std::string> &elems) {
	std::stringstream ss(s);
	std::string item;
	while (std::getline(ss, item, delim)) {
		std::size_t dppos = item.find(':');
		elems[item.substr(0, dppos)] = item.substr(dppos + 1, std::string::npos);
	}
	return elems;
}

std::map<std::string, std::string> Config::configSplit(const std::string &s, char delim) {
	std::map<std::string, std::string> elems;
	configSplit(s, delim, elems);
	return elems;
}

void Config::file_put_contents(const char *path, std::string content) {
	FILE *fp = fopen(path, "w");
	fwrite(content.c_str(), content.size(), 1, fp);
	fclose(fp);
}
