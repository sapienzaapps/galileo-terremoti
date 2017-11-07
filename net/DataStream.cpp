//
// Created by enrico on 11/7/17.
//

#include "DataStream.h"
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/fcntl.h>
#include <errno.h>
#include <string>

int DataStream::readchar() {
	char buf;
	ssize_t r = this->receive(&buf, 1);
	if(r > 0) {
		return buf;
	} else {
		return -1;
	}
}

ssize_t DataStream::readall(uint8_t *buf, size_t len) {
	return this->receive(buf, len);
}

ssize_t DataStream::print(const char* buf) {
	return this->send((void*)buf, strlen(buf));
}

ssize_t DataStream::println(const char* buf) {
	size_t len = strlen(buf);

	std::string line = std::string(buf);
	line.append("\r\n");

	return this->send((void*)line.c_str(), len+2);
}

DataStream::~DataStream() {
}
