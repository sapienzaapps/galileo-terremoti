//
// Created by enrico on 11/7/17.
//

#ifndef GALILEO_TERREMOTI_DATASTREAM_H
#define GALILEO_TERREMOTI_DATASTREAM_H

#include <stdint.h>
#include <stdio.h>
#include <cstdint>
#include <unistd.h>

class DataStream {
public:

	virtual ~DataStream();

	virtual bool doConnect() = 0;
	virtual bool isConnected() = 0;
	virtual bool available() = 0;
	virtual void stop() = 0;

	/**
	 * Send string as TCP data (appending a new-line char '\n')
	 * @param buf String to send
	 * @return Returns the number of byte sent, or -1 if an error occurred
	 */
	ssize_t println(const char* buf);

	/**
	 * Send string as TCP data
	 * @param buf String to send
	 * @return Returns the number of byte sent, or -1 if an error occurred
	 */
	ssize_t print(const char* buf);

	/**
	 * Receive TCP data
	 * @param buf Destination buffer for incoming data
	 * @param len Max data size
	 * @return Returns the number of byte received, or -1 if an error occurred
	 */
	ssize_t readall(uint8_t* buf, size_t len);

	/**
	 * Read only a char from incoming TCP data
	 * @return The character if available, or -1 if an error occurred
	 */
	int readchar();

	virtual ssize_t send(void* buf, size_t size) = 0;
	virtual ssize_t receive(void* buf, size_t maxsize) = 0;
};


#endif //GALILEO_TERREMOTI_DATASTREAM_H
