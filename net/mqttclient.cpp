// Code from Adafruit_MQTT_Library

// The MIT License (MIT)
//
// Copyright (c) 2015 Adafruit Industries
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
#include "mqttclient.h"

bool MQTT_Client::connectServer() {
	client = new Tcp();
	// Grab server name from flash and copy to buffer for name resolution.
	Log::d("Connecting to: %s", servername);
	// Connect and check for success (0 result).
	int r = client->connectTo(servername, portnum);
	Log::d("Connect result: %d", r);
	return r != 0;
}

bool MQTT_Client::disconnectServer() {
	// Stop connection if connected and return success (stop has no indication of
	// failure).
	if (client->connected()) {
		client->stop();
	}
	client = NULL;
	return true;
}

bool MQTT_Client::connected() {
	// Return true if connected, false if not connected.
	return client != NULL && client->connected();
}

uint16_t MQTT_Client::readPacket(uint8_t *buffer, uint16_t maxlen,
								 int16_t timeout) {
	/* Read data until either the connection is closed, or the idle timeout is reached. */
	uint16_t len = 0;
	int16_t t = timeout;

	while (client->connected() && (timeout >= 0)) {
		//DEBUG_PRINT('.');
		while (client->available()) {
			//DEBUG_PRINT('!');
			char c = client->readchar();
			timeout = t;  // reset the timeout
			buffer[len] = c;
			//Log::d((uint8_t)c, HEX);
			len++;
			if (len == maxlen) {  // we read all we want, bail
				Log::d("Read data:\t");
				DEBUG_PRINTBUFFER(buffer, len);
				return len;
			}
		}
		timeout -= MQTT_CLIENT_READINTERVAL_MS;
		Utils::delay(MQTT_CLIENT_READINTERVAL_MS);
	}
	return len;
}

bool MQTT_Client::sendPacket(uint8_t *buffer, uint16_t len) {
	ssize_t ret = 0;

	while (len > 0) {
		if (client->connected()) {
			// send 250 bytes at most at a time, can adjust this later based on Client

			uint16_t sendlen = min(len, 250);
			//Serial.print("Sending: "); Serial.println(sendlen);
			ret = client->send(buffer, sendlen);
			Log::d("Client sendPacket returned: %d", ret);
			len -= ret;

			if (ret != sendlen) {
				Log::d("Failed to send packet.");
				return false;
			}
		} else {
			Log::d("Connection failed!");
			return false;
		}
	}
	return true;
}
