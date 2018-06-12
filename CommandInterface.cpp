//
// Created by ebassetti on 23/07/15.
//

#include <stdint.h>
#include <string.h>
#include <netinet/in.h>
#include "common.h"
#include "CommandInterface.h"
#include "Log.h"

Udp CommandInterface::cmdc;

bool CommandInterface::readPacket(PACKET *pkt) {
	byte pktBuffer[PACKET_SIZE];

	memset(pktBuffer, 0, PACKET_SIZE);
	IPaddr src(0);
	unsigned short port = 0;
	ssize_t cread = cmdc.receive(pktBuffer, (size_t) PACKET_SIZE, &src, &port);
	if (cread > 0) {  // if it received a packet

		if (cread == PACKET_SIZE && memcmp("INGV\0", pktBuffer, 5) == 0) {

			pkt->type = (PacketType) pktBuffer[5];
			pkt->source = src;

			return true;
		}
	}
	return false;
}

void CommandInterface::sendPacket(PACKET pkt) {
	byte pktbuf[PACKET_SIZE];
	memset(pktbuf, 0, PACKET_SIZE);

	memcpy(pktbuf, "INGV\0", 5);
	pktbuf[5] = pkt.type;

	if (pkt.type == PKTTYPE_DISCOVERY_REPLY) {

		byte mac[6];
		Config::getMacAddressAsByte(mac);

		memcpy(pktbuf + 6, mac, 6);

		memcpy(pktbuf + 12, SOFTWARE_VERSION, 4);
		memcpy(pktbuf + 16, PLATFORM_TAG, MINVAL(strlen(PLATFORM_TAG), 8));
	}

	cmdc.send(pktbuf, (size_t) PACKET_SIZE, pkt.source, CMD_INTERFACE_PORT);
}

// check if the mobile APP sent a command to the device
void CommandInterface::checkCommandPacket() {

	PACKET pkt;
	if (!CommandInterface::readPacket(&pkt)) {
		return;
	}

	switch (pkt.type) {
		case PKTTYPE_DISCOVERY: // Discovery
		{
			Log::d("DISCOVERY");
			pkt.type = PKTTYPE_DISCOVERY_REPLY;
			sendPacket(pkt);
		}
			break;
		default:
			break;
	}
}

// establish the connection through the CMD_INTERFACE_PORT port to interact with the mobile APP
bool CommandInterface::commandInterfaceInit() {
	bool ret = cmdc.listen(CMD_INTERFACE_PORT);
	cmdc.setNonblocking();
	if (!ret) {
		Log::e("Error during listening");
	}
	return ret;
}
