
#ifndef GALILEO_TERREMOTI_NETWORKMANAGER_H
#define GALILEO_TERREMOTI_NETWORKMANAGER_H

#include <string>
#include <map>
#include <Arduino.h>
#include <Ethernet.h>

class NetworkManager {
public:
	static void init();
	static bool isConnectedToInternet();
	static bool isConnectedToInternet(bool force);
	static void setupAsDHCPClient(uint8_t *mac);
	static void setupStatic(uint8_t *mac, IPAddress staticAddress, IPAddress subnetMask, IPAddress gateway,
							IPAddress dnsHost);
	static void restart();
	static void forceRestart();
private:
	static bool networkSetup;
	static bool connectionAvailable;
	static bool connectionChecked;
	static bool isDhcpClient;
	static uint8_t *mac;
	static IPAddress staticAddress;
	static IPAddress subnetMask;
	static IPAddress gateway;
	static IPAddress dnsHost;
};


#endif //GALILEO_TERREMOTI_NETWORKMANAGER_H
