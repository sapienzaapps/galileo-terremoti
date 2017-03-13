//
// Created by enrico on 12/03/17.
//

#include "SCSAPI.h"
#include "../generic.h"


MQTT_Client *SCSAPI::mqtt = NULL;
MQTT_Subscribe *SCSAPI::mydev = NULL;
byte SCSAPI::buffer[MAXBUFFERSIZE];
unsigned long SCSAPI::lastNTPTime = 0;
unsigned long SCSAPI::lastNTPMillis = 0;

bool SCSAPI::init() {
	if (mqtt != NULL && mqtt->connected()) {
		return true;
	}
	if (mqtt == NULL) {
		mqtt = new MQTT_Client("mqtt.seismocloud.com", 1883, Config::getMacAddress().c_str(), "embedded",
							   "embedded");
	}
	std::string subtopic("device-");
	subtopic.append(Config::getMacAddress());


	memset(buffer, 0, MAXBUFFERSIZE);
	byte j = 0;
	buffer[j] = API_DISCONNECT;
	j++;

	// Device ID
	buffer[j] = 12;
	j++;
	memcpy(buffer + j, Config::getMacAddress().c_str(), 12);
	j += 12;
	mqtt->will("server", buffer, j, 1);

	mydev = new MQTT_Subscribe(mqtt, subtopic.c_str(), 1);
	mqtt->subscribe(mydev);
	return mqtt->connect() == 0;
}

void SCSAPI::alive() {
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

void SCSAPI::tick() {
	if (mqtt->readSubscription(10) == mydev) {
		byte j = 0;
		switch (mydev->lastread[j]) {
			case API_REBOOT:
				// TODO: disconnected
				platformReboot();
				break;
			case API_CFG:
				if (mydev->datalen < 7) {
					break;
				}
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
				break;
		}
	}
}

void SCSAPI::requestTimeUpdate() {
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

unsigned long SCSAPI::getUNIXTime() {
	unsigned long diff = Utils::millis() - SCSAPI::lastNTPMillis;
	return (SCSAPI::lastNTPTime + (diff / 1000));
}

void SCSAPI::terremoto(RECORD *db) {
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
