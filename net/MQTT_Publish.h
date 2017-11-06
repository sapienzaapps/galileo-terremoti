//
// Created by enrico on 11/6/17.
//

#ifndef GALILEO_TERREMOTI_MQTT_PUBLISH_H
#define GALILEO_TERREMOTI_MQTT_PUBLISH_H


#include "MQTT.h"

class MQTT_Publish {
public:
	MQTT_Publish(MQTT *mqttserver, const char *feed, uint8_t qos = 0);

	bool publish(const char *s);

	bool publish(double f, uint8_t precision = 2);  // Precision controls the minimum number of digits after decimal.
	// This might be ignored and a higher precision value sent.
	bool publish(int32_t i);

	bool publish(uint32_t i);

	bool publish(uint8_t *b, uint16_t bLen);

private:
	MQTT *mqtt;
	const char *topic;
	uint8_t qos;
};


#endif //GALILEO_TERREMOTI_MQTT_PUBLISH_H
