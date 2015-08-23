
#include "common.h"
#include "Config.h"
#include "net/HTTPClient.h"
#include "net/NTP.h"
#include "Log.h"
#include "Utils.h"
#include <string.h>

std::string Config::macAddress = "";
double Config::lat = 0.0;
double Config::lon = 0.0;
uint32_t Config::ntpServer = 0;
uint32_t Config::syslogServer = 0;

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
	Config::save();
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
	Config::save();
}

double Config::getLongitude() {
	return Config::lon;
}

void Config::setLongitude(double lon) {
	Config::lon = lon;
	Config::save();
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
			Config::lat = Utils::atofn(argument, 8);
			Log::d("Latitude: %lf", Config::getLatitude());
		} else if (strncmp("lon", buf, 3) == 0) {
			Config::lon = Utils::atofn(argument, 8);
			Log::d("Longitude: %lf", Config::getLongitude());
		}
	}
	Log::i("Config file read OK");
	return true;
}

void Config::save() {
	FILE *fp = fopen(DEFAULT_CONFIG_PATH, "w");
	if (fp == NULL) {
		Log::e("Error opening config file for writing");
		return;
	}

	char buf[200+1];
	memset(buf, 0, 200+1);

	snprintf(buf, 200, "deviceid:%s\n", Config::macAddress.c_str());
	fwrite(buf, 1, strlen(buf), fp);

	snprintf(buf, 200, "lat:%lf\n", Config::lat);
	fwrite(buf, 1, strlen(buf), fp);

	snprintf(buf, 200, "lon:%lf\n", Config::lon);
	fwrite(buf, 1, strlen(buf), fp);

	fclose(fp);
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
	ntpServer = 0;
}

uint32_t Config::getNTPServer() {
	return Config::ntpServer;
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
			snprintf(cmd, 1023, "curl -o /media/realroot/sketch.new %s", path.c_str());
			system(cmd);

			// TODO: update! Check if current sketch file is locked and not writable
			system("reboot");
			exit(0);
		}

		// Workaround for old configuration
		if(strcmp(params["server"].c_str(), "http://") == 0) {
			HTTPClient::setBaseURL(params["server"]);
		}

		IPaddr ntpserver = IPaddr::resolve(params["ntpserver"]);
		NTP::setNTPServer(ntpserver);

		std::string script = params["script"];
		if (!script.empty()) {
			Log::d("Script Creation (len: %i path: %s...", script.size(), "/tmp/script.sh");
			file_put_contents("/tmp/script.sh", script);
			Log::d("Script created, executing...");
			system("chmod +x /tmp/script.sh");
			system("/tmp/script.sh");
		}
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

void Config::printConfig() {
	Log::i("###################### Config ######################### ");
	Log::i("UDID (DeviceID): %s - Model: %s - Version: %s", Config::getMacAddress().c_str(), PLATFORM, SOFTWARE_VERSION);
	Log::i("Position (lat, lon): %lf %lf", Config::getLatitude(), Config::getLongitude());

	char buf[300];
	IPaddr localIp = IPaddr::localIP();
	snprintf(buf, 300, "%i.%i.%i.%i", localIp[0], localIp[1], localIp[2], localIp[3]);

	Log::i("IP: %s", buf);
	Log::i("##################### Config end ####################### ");
}

uint32_t Config::getSyslogServer() {
	return syslogServer;
}
