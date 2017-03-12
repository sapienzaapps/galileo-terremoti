//
// Created by enrico on 12/03/17.
//

#include "SCSAPI.h"


bool SCSAPI::init() {
	if (mqtt != NULL && mqtt->connected()) {
		return true;
	}
	if (mqtt == NULL) {
		mqtt = new MQTT_Client("mqtt.seismocloud.com", 1883, Config::getMacAddress().c_str(), "embedded",
							   "embedded");
	}
	if (mqtt->connect() == 0) {
		std::string subtopic("device-");
		subtopic.append(Config::getMacAddress());
		MQTT_Subscribe mydev = MQTT_Subscribe(mqtt, subtopic.c_str(), 1);
		mqtt->subscribe(&mydev);
		return true;
	} else {
		return false;
	}
}

void SCSAPI::alive() {
	std::string cfg;
	std::map<std::string, std::string> postValues;
	postValues["deviceid"] = Config::getMacAddress();
	postValues["version"] = SOFTWARE_VERSION;
	postValues["memfree"] = Utils::toString(Utils::getFreeRam());
	postValues["uptime"] = Utils::toString(Utils::uptime());
	postValues["model"] = PLATFORM_TAG;
	postValues["sensor"] = Seismometer::getInstance()->getAccelerometerName();
	postValues["avg"] = Utils::toString(Seismometer::getInstance()->getCurrentAVG());
	postValues["stddev"] = Utils::toString(Seismometer::getInstance()->getCurrentSTDDEV());

	uint8_t buffer[MAXBUFFERSIZE];
	memset(buffer, 0, MAXBUFFERSIZE);
	byte j = 0;
	buffer[j] = API_KEEPALIVE;
	j++;

	// Device ID
	buffer[j] = 12;
	j++;
	memcpy(buffer + j, Config::getMacAddress().c_str(), 12);
	j += 12;

	// Model
	buffer[j] = strlen(BUILD_VERSION);
	j++;
	memcpy(buffer + j, BUILD_VERSION, strlen(BUILD_VERSION));
	j += strlen(BUILD_VERSION);

	// Version
	buffer[j] = strlen(SOFTWARE_VERSION);
	j++;
	memcpy(buffer + j, SOFTWARE_VERSION, strlen(SOFTWARE_VERSION));
	j += strlen(SOFTWARE_VERSION);

	mqtt->publish("server", buffer, j);
}
