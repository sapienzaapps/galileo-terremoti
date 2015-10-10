//
// Created by ebassetti on 19/08/15.
//

#ifndef GALILEO_TERREMOTI_UDP_H
#define GALILEO_TERREMOTI_UDP_H

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "IPaddr.h"

/**
 * UDP class - a simple wrapper to syscalls
 */
class Udp {
public:

	/**
	 * Create a new instance without binding
	 */
	Udp();

	/**
	 * Create an UDP socket and bind it to a destination
	 * @param ipaddr Destination IP Address
	 * @param port Destination UDP port
	 */
	Udp(IPaddr ipaddr, unsigned short port);

	/**
	 * Sets UDP socket to non-blocking I/O
	 */
	void setNonblocking();

	/**
	 * Bind UDP socket to ipaddr/port
	 * @param ipaddr Destination IP Address
	 * @param port Destination UDP port
	 * @return True if binding is successful, false otherwise
	 */
	bool connectTo(IPaddr ipaddr, unsigned short port);

	/**
	 * Listen on a local port
	 * @param port Local UDP port
	 * @return True if binding is successful, false otherwise
	 */
	bool listen(unsigned short port);

	/**
	 * Send UDP datagram to IP address
	 * @param buf Data packet to send
	 * @param size Data packet size
	 * @param dstaddr Destination IP Address
	 * @param port Destination port
	 * @return Returns the number of byte sent, or -1 if an error occurred
	 */
	ssize_t send(void *buf, size_t size, IPaddr dstaddr, unsigned short port);

	/**
	 * Receive UDP datagram
	 * @param buf Destination buffer for incoming packet
	 * @param maxsize Max packet size
	 * @param srcaddr Datagram source IP Address (will be populated only if no error is encountered during receive)
	 * @param port Datagram source port (will be populated only if no error is encountered during receive)
	 * @return Returns the number of byte received, or -1 if an error occurred
	 */
	ssize_t receive(void *buf, size_t maxsize, IPaddr *srcaddr, unsigned short *port);

	/**
	 * Close UDP socket
	 */
	void end();
private:
	int fd;
};


#endif //GALILEO_TERREMOTI_UDP_H
