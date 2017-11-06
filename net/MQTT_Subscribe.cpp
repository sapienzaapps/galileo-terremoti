//
// Created by enrico on 11/6/17.
//

#include "MQTT_Subscribe.h"

MQTT_Subscribe::MQTT_Subscribe(MQTT *mqttserver,
							   const char *feed, uint8_t q) {
	mqtt = mqttserver;
	topic = feed;
	qos = q;
	datalen = 0;
	callback_uint32t = 0;
	callback_buffer = 0;
	callback_double = 0;
//	callback_io = 0;
//	io_mqtt = 0;
}

void MQTT_Subscribe::setCallback(SubscribeCallbackUInt32Type cb) {
	callback_uint32t = cb;
}

void MQTT_Subscribe::setCallback(SubscribeCallbackDoubleType cb) {
	callback_double = cb;
}

void MQTT_Subscribe::setCallback(SubscribeCallbackBufferType cb) {
	callback_buffer = cb;
}

//void MQTT_Subscribe::setCallback(IO_MQTT *io, SubscribeCallbackIOType cb) {
//	callback_io = cb;
//	io_mqtt = io;
//}

void MQTT_Subscribe::removeCallback(void) {
	callback_uint32t = 0;
	callback_buffer = 0;
	callback_double = 0;
//	callback_io = 0;
//	io_mqtt = 0;
}
