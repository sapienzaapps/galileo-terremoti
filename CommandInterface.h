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
	PKTTYPE_DISCOVERY_REPLY = 2
} PacketType;

typedef struct _PACKET {
	PacketType type;
    // My IP Address
	IPaddr source;
	uint32_t uptime;
} PACKET;

class CommandInterface {
public:
	/**
	 * Check if a command packet is received, if so, execute that
	 */
	static void checkCommandPacket();

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
