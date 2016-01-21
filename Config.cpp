
#include "common.h"
#include "Config.h"
#include "net/HTTPClient.h"
#include "net/NTP.h"
#include "Log.h"
#include "Utils.h"
#include "LED.h"
#include "generic.h"
#include <string.h>

std::string Config::macAddress = "";
double Config::lat = 0.0;
double Config::lon = 0.0;
uint32_t Config::syslogServer = 0;

bool Config::hasMACAddress() {
	return !Config::macAddress.empty();
}

bool Config::hasPosition() {
	return Config::lat != 0.0 && Config::lon != 0.0;
}

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
	char *arg;
	while (fgets(buf, 100, fp) != NULL) {
		arg = strchr(buf, ':');
		if (arg == NULL) continue;
		arg++;
		std::string argument = std::string(arg);
		argument = Utils::trim(argument, ' ');
		argument = Utils::trim(argument, '\n');
		argument = Utils::trim(argument, '\r');
		if (strncmp("deviceid", buf, 8) == 0) {
			if (argument.size() < 12) continue;
			Config::macAddress = argument;
			Log::d("Device ID: %s", Config::getMacAddress().c_str());
		} else if (strncmp("lat", buf, 3) == 0) {
			Config::lat = Utils::atofn(argument.c_str(), 8);
			Log::d("Latitude: %lf", Config::getLatitude());
		} else if (strncmp("lon", buf, 3) == 0) {
			Config::lon = Utils::atofn(argument.c_str(), 8);
			Log::d("Longitude: %lf", Config::getLongitude());
		}
	}
	fclose(fp);
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

	snprintf(buf, 200, "lat:%f\n", Config::lat);
	fwrite(buf, 1, strlen(buf), fp);

	snprintf(buf, 200, "lon:%f\n", Config::lon);
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
	macAddress = Utils::getInterfaceMAC();
}

bool Config::checkServerConfig() {
	std::string cfg = HTTPClient::getConfig();
	if(!cfg.empty()) {
		std::map<std::string, std::string> params = configSplit(cfg, '|');
		if(params.empty()) {
			return false;
		}

		if(params.count("collector") == 1) {
			std::string collector = params["collector"];
			Collector::getInstance()->start(IPaddr::resolve(collector));
		} else { // TODO: remove in production
			Collector::getInstance()->start(IPaddr::resolve("192.168.1.200"));
		}

		std::string path = params["path"];
		if(!path.empty()) {
			LED::green(false);
			LED::yellow(false);
			LED::red(false);
			LED::setLedBlinking(LED_RED_PIN);
			// Firmware update
			char cmd[1024];
			memset(cmd, 0, 1024);
			snprintf(cmd, 1023, "curl -o /media/realroot/sketch.new %s", path.c_str());
			system(cmd);

			FILE *fp = fopen("/sketch/update.sh", "w");
			if(fp != NULL) {
				memset(cmd, 0, 1024);
				snprintf(cmd, 1023,
						 "#!/bin/bash\nkillall sketch.elf; mv /media/realroot/sketch.new /sketch/sketch.elf; sleep 1; reboot");
				fwrite(cmd, strlen(cmd), 1, fp);
				fclose(fp);

				system("/bin/bash /sketch/update.sh");
				while(1) {};
			}
			platformReboot();
		}

		// Workaround for old configuration
		if(strcmp(params["server"].c_str(), "http://") == 0) {
			HTTPClient::setBaseURL(params["server"]);
		}

		NTP::setNTPServer(params["ntpserver"]);

//		std::string script = params["script"];
//		if (!script.empty()) {
//			Log::i("Script Creation (len: %i path: %s...", script.size(), "/tmp/script.sh");
//			file_put_contents("/tmp/script.sh", script);
//			Log::d("Script: %s", script.c_str());
//			Log::i("Script created, executing...");
//			system("chmod +x /tmp/script.sh");
//			system("/tmp/script.sh");
//			unlink("/tmp/script.sh");
//		}
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
		Log::d("Splitting line: %s", item.c_str());
		std::size_t dppos = item.find(':');
		if(dppos == std::string::npos) {
			Log::d("Find result in NPOS");
			continue;
		}
		elems[item.substr(0, dppos)] = item.substr(dppos + 1, std::string::npos);
	}
	return elems;
}

std::map<std::string, std::string> Config::configSplit(const std::string &s, char delim) {
	Log::d("Splitting %s", s.c_str());
	std::map<std::string, std::string> elems;
	try {
		configSplit(s, delim, elems);
	} catch(std::exception *e) {
		Log::e("Config splitting exception: %s", e->what());
	}
	return elems;
}

void Config::file_put_contents(const char *path, std::string content) {
	FILE *fp = fopen(path, "w");
	if(fp != NULL) {
		fwrite(content.c_str(), content.size(), 1, fp);
		fclose(fp);
	}
}

void Config::printConfig() {
	Log::i("###################### Config ######################### ");

	Log::i("Software version: %s", SOFTWARE_VERSION);
	Log::i("Platform name: %s", PLATFORM_TAG);

	Log::i("DeviceID: %s", Config::getMacAddress().c_str());

	Log::i("Position (lat, lon): %lf %lf", Config::getLatitude(), Config::getLongitude());

	Log::i("IP: %s", IPaddr::localIP().asString().c_str());

	Log::i("Base URL: %s", HTTPClient::getBaseURL().c_str());

#ifdef DEBUG
	Log::d("Build version: %s", BUILD_VERSION);
#endif
	Log::i("##################### Config end ####################### ");
}

uint32_t Config::getSyslogServer() {
	return syslogServer;
}
