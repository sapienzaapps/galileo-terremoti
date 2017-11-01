//
// Created by ebassetti on 19/08/15.
//

#ifndef GALILEO_TERREMOTI_TCP_H
#define GALILEO_TERREMOTI_TCP_H

#include <stdio.h>
#include <string>
#include "IPaddr.h"

/**
 * TCP class - a simple wrapper to syscalls
 */
class Tcp {
public:

	/**
	 * Create a new instance without binding
	 */
	Tcp();

	/**
	 * Create a TCP socket and connect to a destination
	 * @param ipaddr Destination IP Address
	 * @param port Destination TCP port
	 */
	Tcp(IPaddr ipaddr, unsigned short port);

	/**
	 * Connect to a destination
	 * @param ipaddr Destination Hostname
	 * @param port Destination TCP port
	 * @return True if connection is successful, false otherwise
	 */
	bool connectTo(std::string ipaddr, unsigned short port);

	/**
	 * Connect to a destination
	 * @param ipaddr Destination IP
	 * @param port Destination TCP port
	 * @return True if connection is successful, false otherwise
	 */
	bool connectTo(IPaddr ipaddr, unsigned short port);

	/**
	 * Returns if it's connected or not
	 * @return True if it's connected, false otherwise
	 */
	bool connected();

	/**
	 * Send TCP data
	 * @param buf Data to send
	 * @param size Data size
	 * @return Returns the number of byte sent, or -1 if an error occurred
	 */
	ssize_t send(void* buf, size_t size);

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
	 * @param maxsize Max data size
	 * @return Returns the number of byte received, or -1 if an error occurred
	 */
	ssize_t receive(void* buf, size_t maxsize);

	/**
	 * Check if any data is available from socket
	 * @return True if there one o more bytes pending for receive
	 */
	bool available();

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

	/**
	 * Close TCP connection
	 */
	void stop();

	/**
	 *
	 * @param fd
	 * @param set
	 * @param oldflags
	 * @return
	 */
	int setBlocking(int fd, bool set, int oldflags);

private:
	int fd;
};


#endif //GALILEO_TERREMOTI_TCP_H
