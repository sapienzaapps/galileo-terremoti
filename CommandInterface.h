//
// Created by ebassetti on 23/07/15.
//

#ifndef GALILEO_TERREMOTI_COMMANDINTERFACE_H
#define GALILEO_TERREMOTI_COMMANDINTERFACE_H

#include "net/Udp.h"
#include "Seismometer.h"

#define CMD_INTERFACE_PORT 62001
#define PACKET_SIZE 48

/**
 * Packet definition:
 *
 * Offset Size  Only when         Description
 * ==========================================
 * 0      5                       Magic-bytes "INGV\0"
 * 5      1                       Command (see enum below)
 *
 * 6      6      DISCOVERY_REPLY  MAC address (zero'ed if discovery)
 * 12     4      DISCOVERY_REPLY  Version string (not zero terminated)
 * 16     8      DISCOVERY_REPLY  Model ("galileo1", "galileo2", "simulator") not zero terminated
 *
 * 6      4      SENDGPS          Latitude (IEEE 754)
 * 10     4      SENDGPS          Longitude (IEEE 754)
 *
 */

typedef enum {
	PKTTYPE_DISCOVERY = 1,
	PKTTYPE_DISCOVERY_REPLY = 2,
	PKYTYPE_PING = 3,
	PKYTYPE_PONG = 4,
	PKTTYPE_START = 5,
	PKTTYPE_STOP = 6,
	PKTTYPE_SENDGPS = 7,
	PKTTYPE_OK = 8
} PacketType;

typedef struct _PACKET {
	PacketType type;
	IPaddr source;

	float latitude;
	float longitude;
} PACKET;

class CommandInterface {
public:
	static void checkCommandPacket();
	static void sendValues(RECORD *db);
	static bool commandInterfaceInit();
private:
	static bool readPacket(PACKET*);
	static void sendPacket(PACKET);
	static Udp cmdc;
	static IPaddr udpDest;
};


#endif //GALILEO_TERREMOTI_COMMANDINTERFACE_H
