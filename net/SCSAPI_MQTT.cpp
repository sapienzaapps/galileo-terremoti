//
// Created by enrico on 12/03/17.
//

#include "SCSAPI_MQTT.h"
#include "../generic.h"

SCSAPI_MQTT::SCSAPI_MQTT() {
	this->mqtt = NULL;
	this->mydev = NULL;
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

bool SCSAPI_MQTT::alive() {
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
	buffer[j] = strlen(PLATFORM_TAG);
	j++;
	memcpy(buffer + j, PLATFORM_TAG, strlen(PLATFORM_TAG));
	j += strlen(PLATFORM_TAG);

	// Version
	buffer[j] = strlen(SOFTWARE_VERSION);
	j++;
	memcpy(buffer + j, SOFTWARE_VERSION, strlen(SOFTWARE_VERSION));
	j += strlen(SOFTWARE_VERSION);

	if (!mqtt->publish("server", buffer, j)) {
		if (mqtt->connect() != 0) {
			return false;
		}
		mqtt->publish("server", buffer, j);
	}
	return true;
}

void SCSAPI_MQTT::tick() {
	if (mqtt->readSubscription(10) == mydev) {
		byte j = 0;
		switch (mydev->lastread[j]) {
			case API_REBOOT:
				// TODO: disconnected
				this->mqtt->disconnect();
				Log::i("Reboot by server request");
				platformReboot();
				break;
			case API_CFG:
				if (mydev->datalen < 7) {
					break;
				}
				Log::i("Config update from server");
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
				Log::i("Setting sigma to %f", sigma);
				Seismometer::getInstance()->resetLastPeriod();
				break;
			case API_TIMERESP:
				uint32_t lastNTPTime;
				memcpy(&lastNTPTime, mydev->lastread + 1, 4);
				execSystemTimeUpdate(lastNTPTime);
				Log::i("MQTT Time: %d", lastNTPTime);
				break;
			default:
				Log::e("Invalid packet from server: %d", mydev->lastread[j]);
				break;
		}
	}
}

bool SCSAPI_MQTT::requestTimeUpdate() {
	memset(buffer, 0, MAXBUFFERSIZE);
	byte j = 0;
	buffer[j] = API_TIMEREQ;
	j++;

	// Device ID
	buffer[j] = 12;
	j++;
	memcpy(buffer + j, Config::getMacAddress().c_str(), 12);
	j += 12;

	if (!mqtt->publish("server", buffer, j)) {
		if (mqtt->connect() != 0) {
			return false;
		}
		mqtt->publish("server", buffer, j);
	}
	return true;
}

bool SCSAPI_MQTT::terremoto(RECORD *db) {
	memset(buffer, 0, MAXBUFFERSIZE);
	byte j = 0;
	buffer[j] = API_QUAKE;
	j++;

	// Device ID
	buffer[j] = 12;
	j++;
	memcpy(buffer + j, Config::getMacAddress().c_str(), 12);
	j += 12;

	if (!mqtt->publish("server", buffer, j)) {
		if (mqtt->connect() != 0) {
			return false;
		}
		mqtt->publish("server", buffer, j);
	}
	return true;
}

bool SCSAPI_MQTT::ping() {
	Log::d("PING to server");
	memset(buffer, 0, MAXBUFFERSIZE);
	byte j = 0;
	buffer[j] = API_PING;
	j++;

	// Device ID
	buffer[j] = 12;
	j++;
	memcpy(buffer + j, Config::getMacAddress().c_str(), 12);
	j += 12;

	if (!mqtt->publish("server", buffer, j)) {
		if (mqtt->connect() != 0) {
			return false;
		}
		mqtt->publish("server", buffer, j);
	}
	return true;
}

SCSAPI_MQTT::~SCSAPI_MQTT() {
	delete this->mqtt;
	delete this->mydev;
}
