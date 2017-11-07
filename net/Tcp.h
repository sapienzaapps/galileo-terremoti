//
// Created by ebassetti on 19/08/15.
//

#ifndef GALILEO_TERREMOTI_TCP_H
#define GALILEO_TERREMOTI_TCP_H

#include <stdio.h>
#include <string>
#include "IPaddr.h"
#include "DataStream.h"

/**
 * TCP class - a simple wrapper to syscalls
 */
class Tcp : public DataStream {
public:

	/**
	 * Create a TCP socket and connect to a destination
	 * @param hostname Destination host
	 * @param port Destination TCP port
	 */
	Tcp(std::string hostname, uint16_t port);

	/**
	 * Create a TCP socket and connect to a destination
	 * @param ipaddr Destination IP Address
	 * @param port Destination TCP port
	 */
	Tcp(IPaddr ipaddr, uint16_t port);

	~Tcp();

	/**
	 * Connect to a destination
	 * @param ipaddr Destination Hostname
	 * @param port Destination TCP port
	 * @return True if connection is successful, false otherwise
	 */
	bool doConnect() override;

	/**
	 * Returns if it's connected or not
	 * @return True if it's connected, false otherwise
	 */
	bool isConnected() override;

	/**
	 * Send TCP data
	 * @param buf Data to send
	 * @param size Data size
	 * @return Returns the number of byte sent, or -1 if an error occurred
	 */
	ssize_t send(void* buf, size_t size);

	/**
	 * Receive TCP data
	 * @param buf Destination buffer for incoming data
	 * @param maxsize Max data size
	 * @return Returns the number of byte received, or -1 if an error occurred
	 */
	ssize_t receive(void* buf, size_t maxsize);

	/**
	 * Check if any data is available from socket
	 * @return True if there one o more bytes pending for receive
	 */
	bool available() override;

	/**
	 * Close TCP connection
	 */
	void stop() override;

	/**
	 *
	 * @param fd
	 * @param set
	 * @param oldflags
	 * @return
	 */
	int setBlocking(int fd, bool set, int oldflags);

private:
	IPaddr dst;
	uint16_t port;
	int fd;
};


#endif //GALILEO_TERREMOTI_TCP_H
