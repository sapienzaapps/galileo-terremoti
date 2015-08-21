//
// Created by ebassetti on 19/08/15.
//

#include "Udp.h"
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

Udp::Udp() {
	fd = 0;
}

Udp::Udp(IPaddr ipaddr, unsigned short port) {
	fd = 0;
	connectTo(ipaddr, port);
}

ssize_t Udp::send(void *buf, size_t size, IPaddr dstaddr, unsigned short port) {
	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = htonl(dstaddr);

	return sendto(fd, buf, size, 0, (const sockaddr*)&addr, sizeof(addr));
}

ssize_t Udp::receive(void *buf, size_t maxsize, IPaddr *srcaddr, unsigned short *port) {
	struct sockaddr_in sender;
	socklen_t sendsize = sizeof(sender);
	bzero(&sender, sizeof(sender));

	ssize_t p = recvfrom(fd, buf, maxsize, 0, (struct sockaddr*)&sender, &sendsize);

	if(p > 0) {
		if(srcaddr != NULL) {
			srcaddr->setInt(htonl(sender.sin_addr.s_addr));
		}
		if(port != NULL) {
			*port = htons(sender.sin_port);
		}
	}

	return p;
}

bool Udp::connectTo(IPaddr ipaddr, unsigned short port) {
	fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if(fd < 0) {
		printf("Error during socket creation");
		return false;
	}

	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = htonl(ipaddr);

	int c = connect(fd, (struct sockaddr *)&addr, sizeof(addr));
	if(c < 0) {
		// TODO: error
		printf("Error during connect");
		return false;
	}
	return true;
}

void Udp::end() {
	close(fd);
	fd = 0;
}

bool Udp::listen(unsigned short port) {
	fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if(fd < 0) {
		printf("Error during socket creation");
		return false;
	}

	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = 0;

	if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
		printf("%s", strerror(errno));
		return false;
	}
	return true;
}
