//
// Created by ebassetti on 19/08/15.
//

#ifndef GALILEO_TERREMOTI_TCP_H
#define GALILEO_TERREMOTI_TCP_H

#include <stdio.h>
#include <string>
#include "IPaddr.h"

class Tcp {
public:
	Tcp();
	Tcp(IPaddr ipaddr, unsigned short port);
	bool connectTo(std::string ipaddr, unsigned short port);
	bool connectTo(IPaddr ipaddr, unsigned short port);
	bool connected() const;
	ssize_t send(void* buf, size_t size);
	ssize_t println(const char* buf);
	ssize_t print(const char* buf);
	ssize_t receive(void* buf, size_t maxsize);
	bool available();
	ssize_t readall(uint8_t* buf, size_t len);
	int readchar();
	void stop();
private:
	int fd;
};


#endif //GALILEO_TERREMOTI_TCP_H
