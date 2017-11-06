//
// Created by enrico on 11/6/17.
//

#ifndef GALILEO_TERREMOTI_MQTT_SUBSCRIBE_H
#define GALILEO_TERREMOTI_MQTT_SUBSCRIBE_H


#include "MQTT.h"

class MQTT_Subscribe {
public:
	MQTT_Subscribe(MQTT *mqttserver, const char *feedname, uint8_t q = 0);

	void setCallback(SubscribeCallbackUInt32Type callb);

	void setCallback(SubscribeCallbackDoubleType callb);

	void setCallback(SubscribeCallbackBufferType callb);

//	void setCallback(IO_MQTT *io, SubscribeCallbackIOType callb);

	void removeCallback(void);

	const char *topic;
	uint8_t qos;

	uint8_t lastread[SUBSCRIPTIONDATALEN];
	// Number valid bytes in lastread. Limited to SUBSCRIPTIONDATALEN-1 to
	// ensure nul terminating lastread.
	uint16_t datalen;

	SubscribeCallbackUInt32Type callback_uint32t;
	SubscribeCallbackDoubleType callback_double;
	SubscribeCallbackBufferType callback_buffer;
//	SubscribeCallbackIOType callback_io;
//
//	IO_MQTT *io_mqtt;

private:
	MQTT *mqtt;
};


#endif //GALILEO_TERREMOTI_MQTT_SUBSCRIBE_H
