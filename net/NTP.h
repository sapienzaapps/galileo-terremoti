#ifndef ntp_alt_h
#define ntp_alt_h

#include "IPaddr.h"
#include "Udp.h"

class NTP {
public:
	static void init();
	static void setNTPServer(std::string ntpserver);
	static unsigned long getUNIXTime();
	static unsigned long int getUNIXTimeMS();
	static bool sync();
	static int getHour();
	static std::string getNTPServer();
	static IPaddr getLastNTPServer();

private:
	static IPaddr lastNTPServer;
	static std::string ntpserver;
	static Udp udpSocket;
	static time_t unixTimeTS;
	static unsigned long unixTimeUpdate;

	static void execSystemTimeUpdate(time_t epoch);
	static bool sendNTPpacket(IPaddr address);
};

#endif
