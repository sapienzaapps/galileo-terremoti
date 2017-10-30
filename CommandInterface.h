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
	PKTTYPE_GETINFO_REPLY = 12,
	PKTTYPE_RESET = 13,
	PKTTYPE_TRACE = 14
} PacketType;

typedef struct _PACKET {
	PacketType type;
    // My IP Address
	IPaddr source;

    // SYSLOG server (debug only)
	IPaddr syslogServer;

    // Configured Position
	float latitude;
	float longitude;

    // MAC Address / device ID
	byte mac[6];

    // Current threshold value (in m/s^2)
	float threshold;

    // Unix time in milliseconds
	uint32_t uptime;

    // Unix time in milliseconds
	uint32_t unixts;

    // String with length of 4 (padded with space)
	uint8_t softwareVersion[4 + 1];

    // Free RAM (if available)
	uint32_t freeRam;

    // Network latency if available
	float latency;

    // NTP server
	IPaddr ntpServer;

    // HTTP base (deprecated)
	std::string httpBaseAddress;

    // Platform name (raspi, galileo, etc)
	std::string platformName;

    // Accelerometer model/version
	std::string accelerometerName;

    // Sensor probe speed (for debug only)
	uint32_t statProbeSpeed;
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
 * ?      4      GETINFO_REPLY    Accelerometer probe speed
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
	// static void sendValues(float x, float y, float z);

	/**
	 * Init command interface
	 */
	static bool commandInterfaceInit();

private:
    /**
     * Read packet and populate the structure
     * Returns: true if the packet is valid, false otherwise
     */
	static bool readPacket(PACKET *);

    /**
     * Send the packet to the network
     */
	static void sendPacket(PACKET);

	static Udp cmdc;
	// static IPaddr udpDest;
};


#endif //GALILEO_TERREMOTI_COMMANDINTERFACE_H
