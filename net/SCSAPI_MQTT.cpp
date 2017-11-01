//
// Created by enrico on 12/03/17.
//

#include "SCSAPI_MQTT.h"
#include "../generic.h"

SCSAPI_MQTT::SCSAPI_MQTT() {
	this->mqtt = NULL;
	this->mydev = NULL;
	this->lastNTPMillis = 0;
	this->lastNTPTime = 0;
	this->personalTopic = NULL;
	this->clientid = NULL;
}

bool SCSAPI_MQTT::init() {
	if (mqtt != NULL && mqtt->connected()) {
		return true;
	}
	if (mqtt == NULL) {
		size_t clientidlen = Config::getMacAddress().length() * sizeof(char) + 1;
		clientid = (char *)malloc(clientidlen);
		memset(clientid, 0, clientidlen);

		strncpy(clientid, Config::getMacAddress().c_str(), Config::getMacAddress().length());
		mqtt = new MQTT_Client(MQTT_SERVER, MQTT_PORT, clientid, "embedded", "embedded");
	}
	std::string subtopic("device-");
	subtopic.append(Config::getMacAddress());
	personalTopic = (char*)malloc(subtopic.length() * sizeof(char) + 1);
	memset(personalTopic, 0, subtopic.length() * sizeof(char) + 1);
	strncpy(personalTopic, subtopic.c_str(), subtopic.length());


	memset(buffer, 0, MAXBUFFERSIZE);
	byte j = 0;
	buffer[j] = API_DISCONNECT;
	j++;

	// Device ID
	buffer[j] = 12;
	j++;
	memcpy(buffer + j, Config::getMacAddress().c_str(), 12);
	j += 12;
	mqtt->will("server", buffer, j, 1, 0);

	mydev = new MQTT_Subscribe(mqtt, personalTopic, 1);
	mqtt->subscribe(mydev);
	Log::d("Subscribing to %s", personalTopic);
	return mqtt->connect() == 0;
}

void SCSAPI_MQTT::alive() {
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

void SCSAPI_MQTT::tick() {
	if (mqtt->readSubscription(10) == mydev) {
		byte j = 0;
		switch (mydev->lastread[j]) {
			case API_REBOOT:
				// TODO: disconnected
				this->mqtt->disconnect();
				Log::d("Reboot by server request");
				platformReboot();
				break;
			case API_CFG:
				if (mydev->datalen < 7) {
					break;
				}
				Log::d("Config update from server");
				/**
				   Payload (after type):
				   Offset       Byte      Desc
				   1            4         Sigma (IEEE 754)
				   5            1         Host len
				   6            len(host) Hostname
				   6+len(host)  1         Path len
				   7+len(host)  len(path) Script path
				*/
#ifndef DONT_UPDATE
				byte hlen, plen;
				memcpy(&hlen, mydev->lastread + 5, 1);
				if (hlen > 0) {
					memcpy(&plen, mydev->lastread + 6 + hlen, 1);
					if (plen > 0) {
						*(mydev->lastread + 6 + hlen) = 0;
						memset(buffer, 0, MAXBUFFERSIZE);
						memcpy(buffer, mydev->lastread + 6 + hlen + 1, plen);
						std::string path = "http://";
						path += (char *) (mydev->lastread + 6);
						path += "/";
						path += (char *) buffer;
						Log::d("Update from %s", path.c_str());
						platformUpgrade(path);
						platformReboot();
					}
				}
#endif

				float sigma;
				memcpy(&sigma, mydev->lastread + 1, 4);
				Seismometer::getInstance()->setSigmaIter(sigma);
				Log::d("Setting sigma to %f", sigma);
				Seismometer::getInstance()->resetLastPeriod();
				break;
			case API_TIMERESP:
				memcpy(&lastNTPTime, mydev->lastread + 1, 4);
				Log::d("Time: %d", lastNTPTime);
				lastNTPMillis = Utils::millis();
				break;
			default:
				Log::d("Invalid packet from server: %d", mydev->lastread[j]);
				break;
		}
	}
}

void SCSAPI_MQTT::requestTimeUpdate() {
	memset(buffer, 0, MAXBUFFERSIZE);
	byte j = 0;
	buffer[j] = API_TIMEREQ;
	j++;

	// Device ID
	buffer[j] = 12;
	j++;
	memcpy(buffer + j, Config::getMacAddress().c_str(), 12);
	j += 12;

	mqtt->publish("server", buffer, j);
}

unsigned long SCSAPI_MQTT::getUNIXTime() {
	unsigned long diff = Utils::millis() - SCSAPI_MQTT::lastNTPMillis;
	return (SCSAPI_MQTT::lastNTPTime + (diff / 1000));
}

void SCSAPI_MQTT::terremoto(RECORD *db) {
	memset(buffer, 0, MAXBUFFERSIZE);
	byte j = 0;
	buffer[j] = API_QUAKE;
	j++;

	// Device ID
	buffer[j] = 12;
	j++;
	memcpy(buffer + j, Config::getMacAddress().c_str(), 12);
	j += 12;

	mqtt->publish("server", buffer, j);
}

bool SCSAPI_MQTT::ping() {
	memset(buffer, 0, MAXBUFFERSIZE);
	byte j = 0;
	buffer[j] = API_PING;
	j++;

	// Device ID
	buffer[j] = 12;
	j++;
	memcpy(buffer + j, Config::getMacAddress().c_str(), 12);
	j += 12;

	return mqtt->publish("server", buffer, j);
}

SCSAPI_MQTT::~SCSAPI_MQTT() {
	delete this->mqtt;
	delete this->mydev;
}
