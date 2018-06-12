//
// Created by enrico on 11/6/17.
//

#include "MQTT_Subscribe.h"

MQTT_Subscribe::MQTT_Subscribe(const char *feed, uint8_t q) {
	topic = feed;
	qos = q;
}
