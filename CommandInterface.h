//
// Created by ebassetti on 23/07/15.
//

#ifndef GALILEO_TERREMOTI_COMMANDINTERFACE_H
#define GALILEO_TERREMOTI_COMMANDINTERFACE_H

#include "net/Udp.h"
#include "Seismometer.h"

#define CMD_INTERFACE_PORT 62001
#define PACKET_SIZE 252

typedef enum {
	PKTTYPE_DISCOVERY = 1,
	PKTTYPE_DISCOVERY_REPLY = 2,
	PKYTYPE_PING = 3,
	PKYTYPE_PONG = 4,
	PKTTYPE_START = 5,
	PKTTYPE_STOP = 6,
	PKTTYPE_SENDGPS = 7,
	PKTTYPE_OK = 8,
	PKTTYPE_SETSYSLOG = 9,
	PKTTYPE_REBOOT = 10,
	PKTTYPE_GETINFO = 11,
	PKTTYPE_GETINFO_REPLY = 12
} PacketType;

typedef struct _PACKET {
	PacketType type;
	IPaddr source;

	IPaddr syslogServer;

	float latitude;
	float longitude;
	byte mac[6];

	THRESHOLDS thresholds;
	uint32_t uptime;
	uint32_t unixts;
	uint8_t softwareVersion[4+1];
	uint32_t freeRam;
	float latency;
	IPaddr ntpServer;
	std::string httpBaseAddress;
	std::string platformName;
	std::string accelerometerName;
} PACKET;

/**
 * Packet definition:
 *
 * Offset Size  Only when         Description
 * ==========================================
 * 0      5                       Magic-bytes "INGV\0"
 * 5      1                       Command (see enum below)
 *
 * 6      6      DISCOVERY_REPLY  MAC address
 * 12     4      DISCOVERY_REPLY  Version string (not zero terminated)
 * 16     8      DISCOVERY_REPLY  Model ("galileo1", "galileo2", "simulator") not zero terminated
 *
 * 6      6      SENDGPS          MAC address
 * 12     4      SENDGPS          Latitude (IEEE 754)
 * 16     4      SENDGPS          Longitude (IEEE 754)
 *
 * 6      4      SETSYSLOG        Syslog server
 *
 * 6      6      GETINFO_REPLY    MAC Address
 * 12     4      GETINFO_REPLY    Syslog server
 * 16    12      GETINFO_REPLY    Thresholds (X, Y, Z)
 * 28     4      GETINFO_REPLY    Uptime (seconds)
 * 32     4      GETINFO_REPLY    UNIX time
 * 36     4      GETINFO_REPLY    Software version
 * 40     4      GETINFO_REPLY    Free RAM
 * 44     4      GETINFO_REPLY    Latency
 * 48     4      GETINFO_REPLY    NTP Server
 * 52     -      GETINFO_REPLY    HTTP base address (MAX: 170 chars including ZERO)
 * -      -      GETINFO_REPLY    Platform name (+ variant if any) (MAX: 20 chars including ZERO)
 * -      -      GETINFO_REPLY    Accelerometer name (MAX: 10 chars including ZERO)
 * String max size (sum): 200
 *
 */
class CommandInterface {
public:
	/**
	 * Check if a command packet is received, if so, execute that
	 */
	static void checkCommandPacket();

	/**
	 * Send accelerometer values to Android
	 * @param db Accelerometer values
	 */
	static void sendValues(RECORD *db);

	/**
	 * Init command interface
	 */
	static bool commandInterfaceInit();
private:
	static bool readPacket(PACKET*);
	static void sendPacket(PACKET);
	static Udp cmdc;
	static IPaddr udpDest;
};


#endif //GALILEO_TERREMOTI_COMMANDINTERFACE_H
