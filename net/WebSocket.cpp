//
// Created by enrico on 11/6/17.
//

#include <cstring>
#include <netinet/in.h>
#include "WebSocket.h"
#include "../base64.h"
#include "../Log.h"
#include "../Utils.h"
#include "HTTPClient.h"
#include "MQTT.h"

WebSocket::WebSocket(std::string hostname, uint16_t port, std::string path) {
	this->hostname = hostname;
	this->port = port;
	this->path = path;
	this->tcp = NULL;
}

void WebSocket::stop() {
	if (this->tcp != NULL) {
		this->tcp->stop();
		delete this->tcp;
		this->tcp = NULL;
	}
}

bool WebSocket::doConnect() {
	this->stop();

	this->tcp = new Tcp(this->hostname, this->port);
	if (!this->tcp->doConnect()) {
		return false;
	}

	char keyStart[17];
	memset(keyStart, 0, 17);
	for (int i=0; i<16; ++i) {
		keyStart[i] = (char)((random()/RAND_MAX) * 255);
	}

	ssize_t r = this->tcp->print("GET ");
	if (r < 0) {
		delete this->tcp;
		this->tcp = NULL;
		return false;
	}
	this->tcp->print(this->path.c_str());
	this->tcp->println(" HTTP/1.1");
	this->tcp->println("Upgrade: websocket");
	this->tcp->println("Connection: Upgrade");
	this->tcp->println("Pragma: no-cache");
	this->tcp->println("Cache-Control: no-cache");
	this->tcp->print("Host: ");
	this->tcp->print(this->hostname.c_str());
	this->tcp->print(":");
	this->tcp->println(Utils::toString(this->port).c_str());
	this->tcp->print("Sec-WebSocket-Key: ");
	this->tcp->println(base64Encode(keyStart).c_str());
	this->tcp->println("Sec-WebSocket-Protocol: http");
	this->tcp->println("Sec-WebSocket-Version: 13");
	this->tcp->println("");

	// Request sent, wait for reply
	unsigned long reqTime = Utils::millis();
	while (!tcp->available() && (Utils::millis() - reqTime < HTTP_RESPONSE_TIMEOUT_VALUE)) { ; }

	if (tcp->available()) {
		char rBuffer[300 + 1];
		memset(rBuffer, 0, 300 + 1);
		int s = HTTPClient::getLine(*tcp, (uint8_t *) rBuffer, 300);

		if (strncmp(rBuffer, "HTTP/1.1 101", 12) == 0) {
			// Read headers
			do {
				s = HTTPClient::getLine(*tcp, (uint8_t *) rBuffer, 300);
				if (s > 0 && strlen(rBuffer) != 0) {
					char *dppos = strchr(rBuffer, ':');
					*dppos = 0;
					if (*(dppos + 1) == ' ') {
						dppos++;
					}
					dppos++;
					// TODO: better handling headers and response codes
//					resp->headers[std::string(rBuffer)] = std::string(dppos);
				}
			} while (s > 0 && strlen(rBuffer) != 0);
			// Headers END
			return true;
		} else {
			Log::e("MQTT-HTTP malformed reply");
			this->stop();
			return false;
		}
	} else {
		Log::e("MQTT-HTTP request timeout");
		this->stop();
		return false;
	}
}

ssize_t WebSocket::receive(void* buf1, size_t maxsize) {
	byte* buf = (byte*)buf1;
	if (tcp->available()) {
		uint16_t control;
		if (tcp->readall((uint8_t*)&control, 2) < 0) {
			return -1;
		}
		bool fin = (control & 0x8000) > 0;
		int opcode = (control & 0x0F00) >> 8;
		bool hasmask = (control & 0x0080) > 0;
		uint16_t length = htons((uint16_t)(control & 0x007F));
		int i;

		if (fin) {
			// FIN
		}

		if (length == 127) {
			// TODO: implement larger payload
			return -1;
		}
		if (length == 126) {
			tcp->readall((uint8_t*)&length, 2);
			length = htons(length);
		}
		uint8_t mask[4];
		if (hasmask) {
			tcp->readall(mask, 4);
		}
		int maxlen = min(maxsize, length);
		tcp->readall(buf, length);
		for(i = 0; i < length; i++) {
			buf[i] = buf[i] ^ mask[i % 4];
		}
		// Trash remaining bytes
		byte trashcan = 0;
		for (i = maxlen; i < length; i++) {
			tcp->readall(&trashcan, 1);
		}
		switch (opcode) {
			case 0:
				// Continuation (not supported)
				break;
			case 1: // Text
			case 2: // Binary
			case 10: // PONG
				break;
			case 9:
				// PING
				// TODO: send a PONG
				break;
			case 8: // Connection close
			default:
				return -1;
		}
		return length;
	}
	return -1;
}

bool WebSocket::isConnected() {
	return tcp->isConnected();
}

bool WebSocket::available() {
	return tcp->available();
}

ssize_t WebSocket::send(void *buf, size_t size) {
	if (size > 125) return -1;
	// TODO: payload > 125

	byte *cbuf = (byte*)buf;
	// Size + headers + mask key
	byte *pkt = (byte*) malloc(size + 4 + 4);
	uint32_t mask = (uint32_t)rand();
	uint16_t len = htons(size);
	unsigned int i;

	pkt[0] = 0x82;
	pkt[1] = 0x80 | len;
	memcpy(pkt+2, &mask, 4);
	for(i = 0; i < size; i++) {
		pkt[6 + i] = cbuf[i] ^ pkt[2 + (i % 4)];
	}
	ssize_t r = tcp->send(pkt, size + 4 + 4);
	free(pkt);
	return r;
}
