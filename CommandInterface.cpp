//
// Created by ebassetti on 23/07/15.
//

#include <stdint.h>
#include <string.h>
#include "common.h"
#include "CommandInterface.h"
#include "Log.h"

Udp CommandInterface::cmdc;
IPaddr CommandInterface::udpDest(0);

bool CommandInterface::readPacket(PACKET* pkt) {
	byte mac[6];
	Config::getMacAddressAsByte(mac);

	byte pktBuffer[PACKET_SIZE];

	memset(pktBuffer, 0, 48);
	IPaddr src(0);
	unsigned short port = 0;
	ssize_t cread = cmdc.receive(pktBuffer, (size_t)PACKET_SIZE, &src, &port);
	if (cread > 0) {  // if it received a packet

		if(cread == PACKET_SIZE
				&& memcmp("INGV\0", pktBuffer, 5) == 0
				&& (pktBuffer[5] == PKTTYPE_DISCOVERY || memcmp(pktBuffer + 6, mac, 6) == 0)) {

			pkt->type = (PacketType) pktBuffer[5];
			pkt->source = src;

			if (pkt->type == PKTTYPE_SENDGPS) {
				memcpy(&(pkt->latitude), pktBuffer + 16, 4);
				memcpy(&(pkt->longitude), pktBuffer + 20, 4);
			}
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

	byte mac[6];
	Config::getMacAddressAsByte(mac);

	memcpy(pktbuf + 6, mac, 6);

	if(pkt.type == PKTTYPE_DISCOVERY_REPLY) {
		memcpy(pktbuf + 16, SOFTWARE_VERSION, 4);
#if GALILEO_GEN == 1
		memcpy(pktbuf + 20, "galileo1", 8);
#else
		memcpy(pktbuf + 20, "galileo2", 8);
#endif
	}

	cmdc.send(pktbuf, (size_t)PACKET_SIZE, pkt.source, CMD_INTERFACE_PORT);
}

// check if the mobile APP sent a command to the device
void CommandInterface::checkCommandPacket() {

	PACKET pkt;
	if(!CommandInterface::readPacket(&pkt)) {
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
		case PKYTYPE_PING: // Ping
		{
			Log::d("PING");
			pkt.type = PKYTYPE_PONG;
			sendPacket(pkt);
		}
			break;
		case PKTTYPE_START: // Start
		{
			Log::d("START");
			udpDest = pkt.source;
			pkt.type = PKTTYPE_OK;
			sendPacket(pkt);
		}
			break;
		case PKTTYPE_STOP: // Stop
		{
			Log::d("STOP");
			udpDest = 0;
			pkt.type = PKTTYPE_OK;
			sendPacket(pkt);
		}
			break;
		case PKTTYPE_SENDGPS: // GPS Location
		{
			// Reply
			Config::setLongitude(pkt.latitude);
			Config::setLatitude(pkt.longitude);

			Log::d("Location received - latitude: %lf - longitude: %lf", pkt.latitude, pkt.longitude);

			pkt.type = PKTTYPE_OK;
			sendPacket(pkt);
		}
			break;
		default:
			break;
	}
}

// send the accelerometer values to the mobile APP
// TODO: check
void CommandInterface::sendValues(RECORD *db) {
	uint32_t valx = (uint32_t)db->valx;
	uint32_t valy = (uint32_t)db->valy;
	uint32_t valz = (uint32_t)db->valz;
	if (udpDest == 0) {  // if a socket connection with the mobile APP has been established
		byte pktBuffer[PACKET_SIZE];

		memcpy(pktBuffer, &valx, 4);
		memcpy(pktBuffer+4, &valy, 4);
		memcpy(pktBuffer+8, &valz, 4);

		cmdc.send(pktBuffer, 12, udpDest, CMD_INTERFACE_PORT+1);
	}
}

// establish the connection through the CMD_INTERFACE_PORT port to interact with the mobile APP
bool CommandInterface::commandInterfaceInit() {
	bool ret = cmdc.listen(CMD_INTERFACE_PORT);
	if(!ret) {
		Log::e("Error during listening");
	}
	return ret;
}
