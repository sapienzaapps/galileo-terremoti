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

#include "MQTT.h"
#include "WebSocket.h"


// MQTT Definition ////////////////////////////////////////////////////

MQTT::MQTT(const char *server,
		   uint16_t port,
		   const char *cid,
		   const char *user,
		   const char *pass,
		   const char *wspath) {
	servername = server;
	portnum = port;
	clientid = cid;
	username = user;
	password = pass;
	this->wspath = wspath;

	// reset subscriptions
	for (uint8_t i = 0; i < MAXSUBSCRIPTIONS; i++) {
		subscriptions[i] = 0;
	}

	will_topic = 0;
	will_qos = 0;
	will_retain = 0;

	packet_id_counter = 0;
}

int8_t MQTT::connect() {
	// Connect to the server.
	Log::i("Connecting to MQTT server: %s port %d", this->servername, this->portnum);
	if (!connectServer()) {
		// Retry with HTTP
		if (this->wspath != NULL && strlen(this->wspath) > 0) {
			Log::i("MQTT connection failed, retrying with MQTT-over-WebSocket");
			if (!connectServerViaHTTP()) {
				Log::e("MQTT-HTTP failed");
				return -1;
			}
		} else {
			Log::e("MQTT connection failed");
			return -1;
		}
	}

	Log::d("Connected, sending Connect-Command packet");
	// Construct and send connect packet.
	uint16_t len = connectPacket(buffer);
	if (!sendPacket(buffer, len)) {
		Log::e("Connect-Command send failed");
		return -1;
	}

	Log::d("Waiting for Connect-Ack response");
	// Read connect response packet and verify it
	len = readFullPacket(buffer, MAXBUFFERSIZE, CONNECT_TIMEOUT_MS);
	if (len != 4) {
		Log::e("Invalid Connect-Ack response (invalid length)");
		return -1;
	}
	if ((buffer[0] != (MQTT_CTRL_CONNECTACK << 4)) || (buffer[1] != 2)) {
		Log::e("Invalid ACK received (probably a NAK)");
		return -1;
	}
	if (buffer[3] != 0) {
		// ??
		return buffer[3];
	}

	Log::i("Setting up subscriptions");
	// Setup subscriptions once connected.
	for (uint8_t i = 0; i < MAXSUBSCRIPTIONS; i++) {
		// Ignore subscriptions that aren't defined.
		if (subscriptions[i] == 0) continue;

		bool success = false;
		for (uint8_t retry = 0; retry < 3; retry++) { // retry until we get a suback
			// Construct and send subscription packet.
			Log::d("Subscribing %s (QoS %d)", subscriptions[i]->topic, subscriptions[i]->qos);
			len = subscribePacket(buffer, subscriptions[i]->topic, subscriptions[i]->qos);
			if (!sendPacket(buffer, len)) {
				Log::d("Subscribing packet send failed");
				return -1;
			}

			if (MQTT_PROTOCOL_LEVEL < 3) // older versions didn't suback
				break;

			// Check for SUBACK if using MQTT 3.1.1 or higher
			// TODO: The Server is permitted to start sending PUBLISH packets matching the
			// Subscription before the Server sends the SUBACK Packet. (will really need to use callbacks - ada)

			Log::d("Waiting for Subcription ACK");
			if (processPacketsUntil(buffer, MQTT_CTRL_SUBACK, SUBACK_TIMEOUT_MS)) {
				Log::d("Sub-ACK received");
				success = true;
				break;
			}
			Log::e("Sub-ACK Timeout, Subscription Failed");
		}
		if (!success) {
			return -2;
		}
	}
	Log::i("Subscription completed");
	return 0;
}

uint16_t MQTT::processPacketsUntil(uint8_t *buffer, uint8_t waitforpackettype, uint16_t timeout) {
	uint16_t len;
	while ((len = readFullPacket(buffer, MAXBUFFERSIZE, timeout))) {
		// TODO: add subscription reading & call back processing here

		if ((buffer[0] >> 4) == waitforpackettype) {
			return len;
		} else {
			Log::e("Packet dropped (wrong type %d expected %d)", (buffer[0] >> 4), waitforpackettype);
		}
	}
	return 0;
}

uint16_t MQTT::readFullPacket(uint8_t *buffer, uint16_t maxsize, uint16_t timeout) {
	// will read a packet and Do The Right Thing with length
	uint8_t *pbuff = buffer;
	uint16_t rlen;

	// read the packet type:
	rlen = readPacket(pbuff, 1, timeout);
	if (rlen != 1) {
		return 0;
	}

	pbuff++;

	uint32_t value = 0;
	uint32_t multiplier = 1;
	uint8_t encodedByte;

	do {
		rlen = readPacket(pbuff, 1, timeout);
		if (rlen != 1) {
			return 0;
		}
		encodedByte = pbuff[0]; // save the last read val
		pbuff++; // get ready for reading the next byte
		uint32_t intermediate = (uint32_t)(encodedByte & 0x7F);
		intermediate *= multiplier;
		value += intermediate;
		multiplier *= 128;
		if (multiplier > (128UL * 128UL * 128UL)) {
			Log::i("Malformed packet length");
			return 0;
		}
	} while (encodedByte & 0x80);

	Log::d("Received packet length: %d", value);
	if (value == 0) {
		return 0;
	}

	if (value > (maxsize - (pbuff - buffer) - 1)) {
		Log::e("Packet too big for buffer");
		rlen = readPacket(pbuff, (uint16_t)(maxsize - (pbuff - buffer) - 1), timeout);
	} else {
		rlen = readPacket(pbuff, (uint16_t)value, timeout);
	}

	return (uint16_t)((pbuff - buffer) + rlen);
}

bool MQTT::disconnect() {
	if (this->client == NULL) return true;

	// Construct and send disconnect packet.
	uint16_t len = disconnectPacket(buffer);
	if (!sendPacket(buffer, len)) {
		Log::e("Unable to send disconnect packet");
	}

	return disconnectServer();

}

bool MQTT::publish(const char *topic, uint8_t *data, uint16_t bLen, uint8_t qos) {
	// Construct and send publish packet.
	uint16_t len = publishPacket(buffer, topic, data, bLen, qos);
	if (!sendPacket(buffer, len)) {
		return false;
	}

	// If QOS level is high enough verify the response packet.
	if (qos > 0) {
		len = readFullPacket(buffer, MAXBUFFERSIZE, PUBLISH_TIMEOUT_MS);
		if (len != 4)
			return false;
		if ((buffer[0] >> 4) != MQTT_CTRL_PUBACK)
			return false;
		uint16_t packnum = buffer[2];
		packnum <<= 8;
		packnum |= buffer[3];

		// we increment the packet_id_counter right after publishing so inc here too to match
		packnum++;
		if (packnum != packet_id_counter)
			return false;
	}

	return true;
}

bool MQTT::will(const char *topic, byte *payload, size_t len, uint8_t qos, uint8_t retain) {

	if (connected()) {
		Log::e("Will defined after connect");
		return false;
	}

	will_topic = topic;
	memcpy(will_payload, payload, len);
	will_qos = qos;
	will_retain = retain;

	return true;
}

bool MQTT::subscribe(MQTT_Subscribe *sub) {
	uint8_t i;
	// see if we are already subscribed
	for (i = 0; i < MAXSUBSCRIPTIONS; i++) {
		if (subscriptions[i] == sub) {
			Log::e("Already subscribed");
			return true;
		}
	}
	if (i == MAXSUBSCRIPTIONS) { // add to subscriptionlist
		for (i = 0; i < MAXSUBSCRIPTIONS; i++) {
			if (subscriptions[i] == 0) {
				Log::d("Added sub %s", sub->topic);
				subscriptions[i] = sub;
				return true;
			}
		}
	}

	Log::e("no more subscription space :(");
	return false;
}

bool MQTT::unsubscribe(MQTT_Subscribe *sub) {
	uint8_t i;

	// see if we are already subscribed
	for (i = 0; i < MAXSUBSCRIPTIONS; i++) {
		if (subscriptions[i] == sub) {
			Log::d("Found matching subscription and attempting to unsubscribe.");
			// Construct and send unsubscribe packet.
			uint16_t len = unsubscribePacket(buffer, subscriptions[i]->topic);

			// sending unsubscribe failed
			if (!sendPacket(buffer, len))
				return false;

			// if QoS for this subscription is 1 or 2, we need
			// to wait for the unsuback to confirm unsubscription
			if (subscriptions[i]->qos > 0 && MQTT_PROTOCOL_LEVEL > 3) {

				// wait for UNSUBACK
				len = readFullPacket(buffer, MAXBUFFERSIZE, CONNECT_TIMEOUT_MS);

				if ((len != 5) || (buffer[0] != (MQTT_CTRL_UNSUBACK << 4))) {
					return false;  // failure to unsubscribe
				}
			}

			subscriptions[i] = 0;
			return true;
		}
	}

	// subscription not found, so we are unsubscribed
	return true;
}

MQTT_Subscribe *MQTT::readSubscription(uint16_t timeout) {
	uint16_t i, topiclen, datalen;

	// Check if data is available to read.
	uint16_t len = readFullPacket(buffer, MAXBUFFERSIZE, timeout); // return one full packet
	if (!len) {
		return NULL;
	}  // No data available, just quit.
	Log::d("Packet len: %d", len);

	// Parse out length of packet.
	topiclen = buffer[3];
	Log::d("Looking for my subscriptions (msg sub len %d)", topiclen);
	{
		char tmpdump[512] = {0};
		memcpy(tmpdump, (char *) buffer + 4, topiclen);
		Log::d("Message topic: %s", tmpdump);
	}

	// Find subscription associated with this packet.
	for (i = 0; i < MAXSUBSCRIPTIONS; i++) {
		if (subscriptions[i]) {
			// Skip this subscription if its name length isn't the same as the
			// received topic name.
			if (strlen(subscriptions[i]->topic) != topiclen) {
				Log::d("Skipping %s: invalid topic length", subscriptions[i]->topic);
				continue;
			}
			// Stop if the subscription topic matches the received topic. Be careful
			// to make comparison case insensitive.
			if (strncasecmp((char *) buffer + 4, subscriptions[i]->topic, topiclen) == 0) {
				Log::d("Found sub #%d", i);
				break;
			} else {
				Log::d("Skipping different topic: %s", subscriptions[i]->topic);
			}
		}
	}
	if (i == MAXSUBSCRIPTIONS) {
		Log::d("No subscription found");
		return NULL;
	} // matching sub not found ???

	uint8_t packet_id_len = 0;
	uint16_t packetid = 0;
	// Check if it is QoS 1, TODO: we dont support QoS 2 or 0
	if ((buffer[0] & 0x6) == 0x2) {
		packet_id_len = 2;
		packetid = buffer[topiclen + 4];
		packetid <<= 8;
		packetid |= buffer[topiclen + 5];
		Log::d("QoS 1, packet ID: 0x%x%x", buffer[topiclen + 4], buffer[topiclen + 5]);
	} else if ((buffer[0] & 0x6) == 0x0) {
		Log::d("QoS 0");
	} else {
		Log::d("Wrong QoS: %d", (buffer[0] & 0x6));
		return NULL;
	}

	// zero out the old data
	memset(subscriptions[i]->lastread, 0, SUBSCRIPTIONDATALEN);

	datalen = (uint16_t)(len - topiclen - packet_id_len - 4);
	if (datalen > SUBSCRIPTIONDATALEN) {
		datalen = SUBSCRIPTIONDATALEN - 1; // cut it off
	}
	// extract out just the data, into the subscription object itself
	memmove(subscriptions[i]->lastread, buffer + 4 + topiclen + packet_id_len, datalen);
	subscriptions[i]->datalen = datalen;

	if ((MQTT_PROTOCOL_LEVEL > 3) && (buffer[0] & 0x6) == 0x2) {
		Log::d("Sending ACK for current received packet");
		uint8_t ackpacket[4];

		// Construct and send puback packet.
		len = pubackPacket(ackpacket, packetid);
		if (!sendPacket(ackpacket, len)) {
			Log::e("ACK send failed");
		}
	}

	// return the valid matching subscription
	return subscriptions[i];
}

bool MQTT::ping(uint8_t num) {
	while (num--) {
		// Construct and send ping packet.
		uint16_t len = pingPacket(buffer);
		if (!sendPacket(buffer, len))
			continue;

		// Process ping reply.
		processPacketsUntil(buffer, MQTT_CTRL_PINGRESP, PING_TIMEOUT_MS);
		if (buffer[0] == (MQTT_CTRL_PINGRESP << 4)) {
			return true;
		}
	}
	return false;
}

// Packet Generation Functions /////////////////////////////////////////////////

// The current MQTT spec is 3.1.1 and available here:
//   http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html#_Toc398718028
// However this connect packet and code follows the MQTT 3.1 spec here (some
// small differences in the protocol):
//   http://public.dhe.ibm.com/software/dw/webservices/ws-mqtt/mqtt-v3r1.html#connect
uint16_t MQTT::connectPacket(uint8_t *packet) {
	uint8_t *p = packet;
	uint16_t len;

	// fixed header, connection messsage no flags
	p[0] = (MQTT_CTRL_CONNECT << 4) | 0x0;
	p += 2;
	// fill in packet[1] last

#if MQTT_PROTOCOL_LEVEL == 3
	p = stringprint(p, "MQIsdp");
#elif MQTT_PROTOCOL_LEVEL == 4
	p = Utils::stringprint(p, "MQTT");
#else
#error "MQTT level not supported"
#endif

	p[0] = MQTT_PROTOCOL_LEVEL;
	p++;

	// always clean the session
	p[0] = MQTT_CONN_CLEANSESSION;

	// set the will flags if needed
	if (will_topic && will_topic[0] != 0) {

		p[0] |= MQTT_CONN_WILLFLAG;

		if (will_qos == 1)
			p[0] |= MQTT_CONN_WILLQOS_1;
		else if (will_qos == 2)
			p[0] |= MQTT_CONN_WILLQOS_2;

		if (will_retain == 1)
			p[0] |= MQTT_CONN_WILLRETAIN;

	}

	if (username[0] != 0)
		p[0] |= MQTT_CONN_USERNAMEFLAG;
	if (password[0] != 0)
		p[0] |= MQTT_CONN_PASSWORDFLAG;
	p++;

	p[0] = MQTT_CONN_KEEPALIVE >> 8;
	p++;
	p[0] = MQTT_CONN_KEEPALIVE & 0xFF;
	p++;

	if (MQTT_PROTOCOL_LEVEL == 3) {
		p = Utils::stringprint(p, clientid, 23);  // Limit client ID to first 23 characters.
	} else {
		if (clientid[0] != 0) {
			p = Utils::stringprint(p, clientid);
		} else {
			p[0] = 0x0;
			p++;
			p[0] = 0x0;
			p++;
			Log::d("SERVER GENERATING CLIENT ID");
		}
	}

	if (will_topic && will_topic[0] != 0) {
		p = Utils::stringprint(p, will_topic);
		p = Utils::binprint(p, will_payload);
	}

	if (username[0] != 0) {
		p = Utils::stringprint(p, username);
	}
	if (password[0] != 0) {
		p = Utils::stringprint(p, password);
	}

	len = (uint16_t)(p - packet);

	packet[1] = (uint8_t)(len - 2);  // don't include the 2 bytes of fixed header data
	return len;
}


// as per http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html#_Toc398718040
uint16_t MQTT::publishPacket(uint8_t *packet, const char *topic,
							 uint8_t *data, uint16_t bLen, uint8_t qos) {
	uint8_t *p = packet;
	uint16_t len = 0;

	// calc length of non-header data
	len += 2;               // two bytes to set the topic size
	len += strlen(topic); // topic length
	if (qos > 0) {
		len += 2; // qos packet id
	}
	len += bLen; // payload length

	// Now you can start generating the packet!
	p[0] = (uint8_t)(MQTT_CTRL_PUBLISH << 4 | qos << 1);
	p++;

	// fill in packet[1] last
	do {
		uint8_t encodedByte = (uint8_t)(len % 128);
		len /= 128;
		// if there are more data to encode, set the top bit of this byte
		if (len > 0) {
			encodedByte |= 0x80;
		}
		p[0] = encodedByte;
		p++;
	} while (len > 0);

	// topic comes before packet identifier
	p = Utils::stringprint(p, topic);

	// add packet identifier. used for checking PUBACK in QOS > 0
	if (qos > 0) {
		p[0] = (uint8_t)((packet_id_counter >> 8) & 0xFF);
		p[1] = (uint8_t)(packet_id_counter & 0xFF);
		p += 2;

		// increment the packet id
		packet_id_counter++;
	}

	memmove(p, data, bLen);
	p += bLen;
	len = (uint16_t)(p - packet);
	return len;
}

uint16_t MQTT::subscribePacket(uint8_t *packet, const char *topic,
							  uint8_t qos) {
	uint8_t *p = packet;
	uint16_t len;

	p[0] = MQTT_CTRL_SUBSCRIBE << 4 | MQTT_QOS_1 << 1;
	// fill in packet[1] last
	p += 2;

	// packet identifier. used for checking SUBACK
	p[0] = (uint8_t)((packet_id_counter >> 8) & 0xFF);
	p[1] = (uint8_t)(packet_id_counter & 0xFF);
	p += 2;

	// increment the packet id
	packet_id_counter++;

	p = Utils::stringprint(p, topic);

	p[0] = qos;
	p++;

	len = (uint16_t)(p - packet);
	packet[1] = (uint8_t)(len - 2); // don't include the 2 bytes of fixed header data
	return len;
}


uint16_t MQTT::unsubscribePacket(uint8_t *packet, const char *topic) {

	uint8_t *p = packet;
	uint16_t len;

	p[0] = MQTT_CTRL_UNSUBSCRIBE << 4 | 0x1;
	// fill in packet[1] last
	p += 2;

	// packet identifier. used for checking UNSUBACK
	p[0] = (uint8_t)((packet_id_counter >> 8) & 0xFF);
	p[1] = (uint8_t)(packet_id_counter & 0xFF);
	p += 2;

	// increment the packet id
	packet_id_counter++;

	p = Utils::stringprint(p, topic);

	len = (uint16_t)(p - packet);
	packet[1] = (uint8_t)(len - 2); // don't include the 2 bytes of fixed header data
	return len;

}

uint16_t MQTT::pingPacket(uint8_t *packet) {
	packet[0] = MQTT_CTRL_PINGREQ << 4;
	packet[1] = 0;
	return 2;
}

uint16_t MQTT::pubackPacket(uint8_t *packet, uint16_t packetid) {
	packet[0] = MQTT_CTRL_PUBACK << 4;
	packet[1] = 2;
	packet[2] = (uint8_t)(packetid >> 8);
	packet[3] = (uint8_t)packetid;
	return 4;
}

uint16_t MQTT::disconnectPacket(uint8_t *packet) {
	packet[0] = MQTT_CTRL_DISCONNECT << 4;
	packet[1] = 0;
	return 2;
}

bool MQTT::connectServer() {
	this->disconnect();
	client = new Tcp(servername, portnum);

	Log::d("Connecting to: %s", servername);

	// Connect and check for success (0 result).
	int r = client->doConnect();
	Log::d("Connect result: %d", r);
	return r != 0;
}

bool MQTT::connectServerViaHTTP() {
	if (wspath == NULL || strlen(wspath) == 0) return false;
	this->disconnect();
	client = new WebSocket(servername, portnum, wspath);

	Log::d("Connecting to: %s", servername);

	// Connect and check for success (0 result).
	int r = client->doConnect();
	Log::d("Connect result: %d", r);
	return r != 0;
}

bool MQTT::disconnectServer() {
	if (client == NULL) {
		return true;
	}
	// Stop connection if connected and return success (stop has no indication of
	// failure).
	if (client->isConnected()) {
		client->stop();
	}
	client = NULL;
	return true;
}

bool MQTT::connected() {
	// Return true if connected, false if not connected.
	return client != NULL && client->isConnected();
}

uint16_t MQTT::readPacket(uint8_t *buffer, uint16_t maxlen,
								 int16_t timeout) {
	/* Read data until either the connection is closed, or the idle timeout is reached. */
	uint16_t len = 0;
	int16_t t = timeout;

	while (client->isConnected() && (timeout >= 0)) {
		while (client->available()) {
			int c = client->readchar();
			if (c >= 0) {
				timeout = t;  // reset the timeout
				buffer[len] = (uint8_t) c;
				len++;
				if (len == maxlen) {  // we read all we want, bail
					return len;
				}
			}
		}
		timeout -= MQTT_CLIENT_READINTERVAL_MS;
		Utils::delay(MQTT_CLIENT_READINTERVAL_MS);
	}
	return len;
}

bool MQTT::sendPacket(uint8_t *buffer, uint16_t len) {
	if (client == NULL) return false;
	ssize_t ret = 0;

	while (len > 0) {
		if (client->isConnected()) {
			// send 250 bytes at most at a time, can adjust this later based on Client

			uint16_t sendlen = (uint16_t)min(len, 250);
			//Serial.print("Sending: "); Serial.println(sendlen);
			ret = client->send(buffer, sendlen);
			Log::d("Client sendPacket returned: %d", ret);
			len -= ret;

			if (ret != sendlen) {
				Log::e("Failed to send packet.");
				return false;
			}
		} else {
			Log::e("Connection failed!");
			return false;
		}
	}
	return true;
}
