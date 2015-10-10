#ifndef ntp_alt_h
#define ntp_alt_h

#include "IPaddr.h"
#include "Udp.h"

/**
 * NTP implementation
 */
class NTP {
public:
	/**
	 * Init NTP class
	 */
	static void init();

	/**
	 * Set NTP server
	 * Note: if this value is an hostname, at every sync a DNS lookup is done to discover IP address
	 */
	static void setNTPServer(std::string ntpserver);

	/**
	 * Get time as UNIX timestamp (eg. seconds since Jan 1 1970 UTC)
	 * @return UNIX time
	 */
	static unsigned long getUNIXTime();

	/**
	 * Get time as UNIX timestamp (eg. seconds since Jan 1 1970 UTC) with milliseconds precision
	 * @return UNIX time with millisecond precision
	 */
	static unsigned long int getUNIXTimeMS();

	/**
	 * Sync with NTP server
	 * @return True if sync is successful, false otherwise
	 */
	static bool sync();

	/**
	 * Get the current hour (in 24-hour format)
	 * @return Current hour (in 24-hour format)
	 */
	static int getHour();

	/**
	 * Returns the current NTP server hostname used for sync
	 * Note: if this value is an hostname, at every sync a DNS lookup is done to discover IP address
	 * @return NTP server configured
	 */
	static std::string getNTPServer();

	/**
	 * Returns the last NTP server used to sync (as IP address)
	 * @return Last NTP server contacted
	 */
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
