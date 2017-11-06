//
// Created by enrico on 11/6/17.
//

#ifndef GALILEO_TERREMOTI_MQTT_SUBSCRIBE_H
#define GALILEO_TERREMOTI_MQTT_SUBSCRIBE_H

#include <stdlib.h>
#include <stdint.h>

// how much data we save in a subscription object
// eg max-subscription-payload-size
#define SUBSCRIPTIONDATALEN 100

class MQTT_Subscribe {
public:
	MQTT_Subscribe(const char *feedname, uint8_t q = 0);
	const char *topic;
	uint8_t qos;

	uint8_t lastread[SUBSCRIPTIONDATALEN];
	// Number valid bytes in lastread. Limited to SUBSCRIPTIONDATALEN-1 to
	// ensure nul terminating lastread.
	uint16_t datalen;
};

#endif //GALILEO_TERREMOTI_MQTT_SUBSCRIBE_H
