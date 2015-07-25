//
// Created by ebassetti on 23/07/15.
//

#include <stdint.h>
#include <string.h>
#include <BitsAndBytes.h>
#include "common.h"
#include "CommandInterface.h"
#include "Log.h"

EthernetUDP CommandInterface::cmdc;
IPAddress CommandInterface::udpDest = (in_addr_t) 0;

bool CommandInterface::readPacket(PACKET* pkt) {
	byte mac[6];
	Config::getMacAddressAsByte(mac);

	byte pktBuffer[PACKET_SIZE];

	if (cmdc.parsePacket() > 0) {  // if it received a packet
		memset(pktBuffer, 0, 48);
		int cread = cmdc.read(pktBuffer, PACKET_SIZE);

		if(cread == PACKET_SIZE
				&& memcmp("INGV\0", pktBuffer, 5) == 0
				&& (pktBuffer[5] == PKTTYPE_DISCOVERY || memcmp(pktBuffer + 6, mac, 6) == 0)) {

			pkt->type = (PacketType) pktBuffer[5];
			pkt->source = (uint32_t) pktBuffer[12] << 24
						  | (uint32_t) pktBuffer[13] << 16
						  | (uint32_t) pktBuffer[14] << 8
						  | pktBuffer[15];
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
	cmdc.beginPacket(pkt.source, CMD_INTERFACE_PORT);
	cmdc.write(pktbuf, PACKET_SIZE);
	cmdc.endPacket();
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
			udpDest = (in_addr_t) 0;
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
	if (udpDest._sin.sin_addr.s_addr == (in_addr_t)0) {  // if a socket connection with the mobile APP has been established
		byte pktBuffer[PACKET_SIZE];

		cmdc.beginPacket(udpDest, CMD_INTERFACE_PORT+1);

		memcpy(pktBuffer, &valx, 4);
		memcpy(pktBuffer+4, &valy, 4);
		memcpy(pktBuffer+8, &valz, 4);

		cmdc.write(pktBuffer, 12);
		cmdc.endPacket();
	}
}

// establish the connection through the CMD_INTERFACE_PORT port to interact with the mobile APP
void CommandInterface::commandInterfaceInit() {
	cmdc.begin(CMD_INTERFACE_PORT);
}
