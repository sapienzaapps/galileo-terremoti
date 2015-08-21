
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

private:
	static bool connectionAvailable;
	static bool connectionChecked;
};


#endif //GALILEO_TERREMOTI_NETWORKMANAGER_H
