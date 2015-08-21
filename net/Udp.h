//
// Created by ebassetti on 19/08/15.
//

#ifndef GALILEO_TERREMOTI_UDP_H
#define GALILEO_TERREMOTI_UDP_H

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "IPaddr.h"

class Udp {
public:
	Udp();
	Udp(IPaddr ipaddr, unsigned short port);
	bool connectTo(IPaddr ipaddr, unsigned short port);
	bool listen(unsigned short port);
	ssize_t send(void *buf, size_t size, IPaddr dstaddr, unsigned short port);
	ssize_t receive(void *buf, size_t maxsize, IPaddr *srcaddr, unsigned short *port);
	void end();
private:
	int fd;
};


#endif //GALILEO_TERREMOTI_UDP_H
