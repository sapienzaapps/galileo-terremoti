#ifndef ntp_alt_h
#define ntp_alt_h

#include <sstream>
#include <EthernetUdp.h>
#include <IPAddress.h>

class NTP {
public:
	static void initNTP();
	static void setNTPServer(IPAddress ntpserver);
	static unsigned long getUNIXTime();
	static unsigned long int getUNIXTimeMS();
	static bool sync();
	static int getHour();
private:
	static IPAddress ntpserver;
	static EthernetUDP udpSocket;
	static unsigned long unixTimeTS;
	static unsigned long unixTimeUpdate;

	static void execSystemTimeUpdate(unsigned long epoch);
	static void sendNTPpacket(IPAddress &address);
};

#endif
