//
// Created by ebassetti on 19/08/15.
//

#include "Tcp.h"
#include "../Log.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <sys/fcntl.h>
#include <errno.h>

Tcp::Tcp() {
	signal(SIGPIPE, SIG_IGN);
	fd = 0;
}

Tcp::Tcp(IPaddr ipaddr, unsigned short port) {
	signal(SIGPIPE, SIG_IGN);
	fd = 0;
	connectTo(ipaddr, port);
}

ssize_t Tcp::send(void *buf, size_t size) {
	if(fd <= 0) return -1;
	return write(fd, buf, size);
}

ssize_t Tcp::print(const char* buf) {
	if(fd <= 0) return -1;
	return send((void*)buf, strlen(buf));
}

ssize_t Tcp::println(const char* buf) {
	if(fd <= 0) return -1;
	size_t len = strlen(buf);

	std::string line = std::string(buf);
	line.append("\r\n");

	return send((void*)line.c_str(), len+2);
}

ssize_t Tcp::receive(void *buf, size_t maxsize) {
	if(fd <= 0) return -1;
	return read(fd, buf, maxsize);
}

bool Tcp::available() {
	if(fd <= 0) return false;
	char buf;
	int oldflags = setBlocking(fd, true, 0);
	ssize_t peekdata = recv(fd, &buf, 1, MSG_PEEK);
	setBlocking(fd, false, oldflags);
	return peekdata > 0;
}

bool Tcp::connectTo(std::string hostname, unsigned short port) {
	return connectTo(IPaddr::resolve(hostname), port);
}

bool Tcp::connectTo(IPaddr ipaddr, unsigned short port) {
	fd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(fd < 0) {
		printf("Error during socket creation");
		fd = 0;
		return false;
	}

	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = htonl(ipaddr);

	fd_set set;
	FD_ZERO(&set);
	FD_SET(fd, &set);

	int flags = fcntl(fd, F_GETFL, 0);
	fcntl(fd, F_SETFL, flags | O_NONBLOCK);

	int c = connect(fd, (struct sockaddr *)&addr, sizeof(addr));
	if(c < 0 && errno != EINPROGRESS) {
		// TODO: error
		Log::e("Error during connect: %s", strerror(errno));
		fd = 0;
		return false;
	}

	struct timeval  timeout;
	timeout.tv_sec = 5;
	timeout.tv_usec = 0;

	int status = select(fd+1, NULL, &set, NULL, &timeout);

	if(status == -1) {
		Log::e("Error during TCP select(): %s", strerror(errno));
		fd = 0;
		return false;
	} else if(status == 0) {
		Log::e("Timeout during connect to %s", ipaddr.asString().c_str());
		close(fd);
		shutdown(fd, SHUT_RDWR);
		fd = 0;
		return false;
	}

	// Restore blocking mode
	fcntl(fd, F_SETFL, flags);

	return connected();
}

ssize_t Tcp::readall(uint8_t *buf, size_t len) {
	if(fd <= 0) return -1;
	return read(fd, buf, len);
}

void Tcp::stop() {
	if(fd <= 0) return;
	shutdown(fd, SHUT_RDWR);
	close(fd);
	fd = 0;
}

bool Tcp::connected() {
	bool ret = false;
	if(fd > 0) {
		ret = true;
		int oldflags = setBlocking(fd, true, 0);
		char buf;
		ssize_t err = recv(fd, &buf, 1, MSG_PEEK);
		if(err == 0) {
			ret = false;
		}
		setBlocking(fd, false, oldflags);
	}
	return ret;
}

int Tcp::setBlocking(int fd, bool set, int oldflags) {
	if (set) {
		// Nonblocking
		int flags = fcntl(fd, F_GETFL, 0);
		fcntl(fd, F_SETFL, flags | O_NONBLOCK);
		return flags;
	} else {
		// Restore blocking mode
		return fcntl(fd, F_SETFL, oldflags);
	}
}

int Tcp::readchar() {
	if(fd <= 0) return -1;
	char buf;
	ssize_t r = read(fd, &buf, 1);
	if(r > 0) {
		return buf;
	} else {
		return -1;
	}
}
