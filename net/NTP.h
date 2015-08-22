#ifndef ntp_alt_h
#define ntp_alt_h

#include "IPaddr.h"
#include "Udp.h"

class NTP {
public:
	static void init();
	static void setNTPServer(IPaddr ntpserver);
	static unsigned long getUNIXTime();
	static unsigned long int getUNIXTimeMS();
	static bool sync();
	static int getHour();
private:
	static IPaddr ntpserver;
	static Udp udpSocket;
	static time_t unixTimeTS;
	static unsigned long unixTimeUpdate;

	static void execSystemTimeUpdate(time_t epoch);
	static bool sendNTPpacket(IPaddr address);
};

#endif
