
#ifndef GALILEO_TERREMOTI_NETWORKMANAGER_H
#define GALILEO_TERREMOTI_NETWORKMANAGER_H

#include <string>
#include <map>
#include "IPaddr.h"

/**
 * Some useful network functions
 */
class NetworkManager {
public:
	/**
	 * Do a latency calculation to 8.8.8.8
	 * @return Network latency
	 */
	static float latency();

	/**
	 * Send an ICMP PING to address to check if it's alive
	 * @param address Destination address
	 * @param waitms Timeout for ICMP PONG (response)
	 * @param sequenceNumber PING sequence number (if used more than one time in short time; i.e. in a loop)
	 * @return True if ICMP PONG is received in timeout, false otherwise
	 */
	static bool ping(IPaddr address, unsigned int waitms, uint16_t sequenceNumber);

private:
	static unsigned short checksum(void *b, int len);
};

#if (defined(OPENBSD) || defined(FREEBSD) ||defined(__APPLE__) || defined(__darwin__)) && !defined(SOL_IP)
#define SOL_IP IPPROTO_IP  /* SOL_IP is not defined on OSX Lion */
#endif /* TARGET_DARWIN && !SOL_IP */

#endif //GALILEO_TERREMOTI_NETWORKMANAGER_H
