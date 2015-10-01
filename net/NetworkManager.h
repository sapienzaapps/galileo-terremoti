
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


#endif //GALILEO_TERREMOTI_NETWORKMANAGER_H
