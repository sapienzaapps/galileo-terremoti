
#ifndef GALILEO_TERREMOTI_NETWORKMANAGER_H
#define GALILEO_TERREMOTI_NETWORKMANAGER_H

#include <string>
#include <map>
#include "IPaddr.h"

class NetworkManager {
public:
	static void init();
	static bool isConnectedToInternet();
	static bool isConnectedToInternet(bool force);
	static float getLastLatency();
	static float latency();
	static bool ping(IPaddr address, unsigned int waitms, uint16_t sequenceNumber);

private:
	static bool connectionAvailable;
	static bool connectionChecked;
	static float lastLatency;
	static unsigned short checksum(void *b, int len);
};

#if (defined(OPENBSD) || defined(FREEBSD) ||defined(__APPLE__) || defined(__darwin__)) && !defined(SOL_IP)
#define SOL_IP IPPROTO_IP  /* SOL_IP is not defined on OSX Lion */
#endif /* TARGET_DARWIN && !SOL_IP */

#endif //GALILEO_TERREMOTI_NETWORKMANAGER_H
