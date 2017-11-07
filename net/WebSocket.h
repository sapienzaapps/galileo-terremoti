//
// Created by enrico on 11/6/17.
//

#ifndef GALILEO_TERREMOTI_WEBSOCKET_H
#define GALILEO_TERREMOTI_WEBSOCKET_H

#include "Tcp.h"
#include "../common.h"

class WebSocket : public DataStream {
public:
	WebSocket(std::string hostname, uint16_t port, std::string path);

	bool doConnect();

	bool isConnected();

	bool available() override;

	ssize_t send(void* buf, size_t size);

	ssize_t receive(void* buf, size_t maxsize);

	void stop();

private:
	std::string hostname;
	std::string path;
	uint16_t port;
	Tcp *tcp;
};

#endif //GALILEO_TERREMOTI_WEBSOCKET_H
