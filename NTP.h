#ifndef ntp_alt_h
#define ntp_alt_h

#include <Arduino.h>
#include <Ethernet.h>
#include <EthernetUdp.h>
#include <sstream>
#include "NetworkManager.h"

unsigned long fixword(byte b1, byte b2);

class NTP {
public:
	static void initNTP();
	static void setNTPServer(IPAddress ntpserver);
	static unsigned long getUNIXTime();
	static unsigned long int getUNIXTimeMS();
	static bool sync();
private:
	static IPAddress ntpserver;
	static EthernetUDP udpSocket;
	static unsigned long unixTimeTS;
	static unsigned long unixTimeUpdate;

	static bool dataPacket();
	static void execSystemTimeUpdate(unsigned long epoch);
	static void sendNTPpacket(IPAddress &address);
};

#endif
