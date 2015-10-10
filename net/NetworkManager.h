
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
	 * Init networking
	 */
	static void init();

	/**
	 * Returns if device is connected to internet (by sending an ICMP PING to 8.8.8.8)
	 * @param force If true, a test is enforced; if false, last test status is returned if valid
	 * @return True if device is connected to internet, false otherwise
	 */
	static bool isConnectedToInternet(bool force);

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
	static bool connectionAvailable;
	static bool connectionChecked;
	static unsigned short checksum(void *b, int len);
};

#if (defined(OPENBSD) || defined(FREEBSD) ||defined(__APPLE__) || defined(__darwin__)) && !defined(SOL_IP)
#define SOL_IP IPPROTO_IP  /* SOL_IP is not defined on OSX Lion */
#endif /* TARGET_DARWIN && !SOL_IP */

#endif //GALILEO_TERREMOTI_NETWORKMANAGER_H
