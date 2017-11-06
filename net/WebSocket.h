//
// Created by enrico on 11/6/17.
//

#ifndef GALILEO_TERREMOTI_WEBSOCKET_H
#define GALILEO_TERREMOTI_WEBSOCKET_H

#include "Tcp.h"
#include "../common.h"

class WebSocket {
public:
	WebSocket(std::string hostname, uint16_t port, std::string path);

	bool connect();

	bool connected();

	bool available();

	ssize_t send(void* buf, size_t size);

	int readMessage(byte** msg);

	void disconnect();

private:
	std::string hostname;
	std::string path;
	uint16_t port;
	Tcp *tcp;
};

#endif //GALILEO_TERREMOTI_WEBSOCKET_H
