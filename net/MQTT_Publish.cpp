//
// Created by enrico on 11/6/17.
//

#include "MQTT_Publish.h"

MQTT_Publish::MQTT_Publish(MQTT *mqttserver,
						   const char *feed, uint8_t q) {
	mqtt = mqttserver;
	topic = feed;
	qos = q;
}

bool MQTT_Publish::publish(int32_t i) {
	char payload[12];
	Utils::ltoa(i, payload, 10);
	return mqtt->publish(topic, payload, qos);
}

/*
bool MQTT_Publish::publish(uint32_t i) {
    char payload[11];
    ultoa(i, payload, 10);
    return mqtt->publish(topic, payload, qos);
}

bool MQTT_Publish::publish(double f, uint8_t precision) {
    char payload[41];  // Need to technically hold float max, 39 digits and minus sign.
    dtostrf(f, 0, precision, payload);
    return mqtt->publish(topic, payload, qos);
}*/

bool MQTT_Publish::publish(const char *payload) {
	return mqtt->publish(topic, payload, qos);
}

//publish buffer of arbitrary length
bool MQTT_Publish::publish(uint8_t *payload, uint16_t bLen) {

	return mqtt->publish(topic, payload, bLen, qos);
}