#include "localstream.h"
#include "Log.h"
#include "commons.h"

byte _pktBuffer[CONTROLPKTSIZE];
EthernetUDP _cmdc;
uint32_t _udpDest = (uint32_t) 0;
IPAddress _udpTemp(0, 0, 0, 0);


union ArrayToInteger {
	byte array[4];
	uint32_t integer;
};

// check if the mobile APP sent a command to the device
void checkCommandPacket() {

	byte mac[6];
	Config::getMacAddressAsByte(mac);

	if (_cmdc.parsePacket() > 0) {  // if it received a packet
		memset(_pktBuffer, 0, 48);
		_cmdc.read(_pktBuffer, CONTROLPKTSIZE);
		// if its a packet from the mobile APP (contains the packet ID: INGV):
		// if the device must be discovered or has already been discovered
		if (memcmp("INGV\0", _pktBuffer, 5) == 0 &&
			(_pktBuffer[5] == 1 || memcmp(_pktBuffer + 6, mac, 6) == 0)) {

			Log::d("%s", _pktBuffer);

			byte command = _pktBuffer[5];

			/* CONVERTING IP ADDRESS FROM CHAR TO BYTE */
			uint32_t IPinteger =
					(uint32_t) _pktBuffer[12] << 24
					| (uint32_t) _pktBuffer[13] << 16
					| (uint32_t) _pktBuffer[14] << 8
					| _pktBuffer[15];
			_udpTemp = IPinteger;

			switch (command) {
				case 1: // Discovery
				{
					Log::d("DISCOVERY");
					_pktBuffer[5] = 1;

					// store the MAC address of the device inside the packet to let the APP know
					memcpy(_pktBuffer + 6, mac, 6);

					// Store software version
					memcpy(_pktBuffer + 35, SOFTWARE_VERSION, 4);

					memcpy(_pktBuffer + 39, ARDUINO_MODEL, 8);

					_pktBuffer[47] = '\0';

					_cmdc.beginPacket(_udpTemp, 62001);
					_cmdc.write(_pktBuffer, CONTROLPKTSIZE + 4);
					_cmdc.endPacket();
				}
					break;
				case 2: // Ping
				{
					Log::d("PING");  // start sending packets to the mobile APP
					// Reply
					_pktBuffer[5] = 3;
					_cmdc.beginPacket(_udpTemp, 62001);
					_cmdc.write(_pktBuffer, CONTROLPKTSIZE);
					_cmdc.endPacket();
					Log::d("PONG");
				}
					break;
				case 4: // Start
				{
					Log::d("START");  // start the socket connection with the mobile APP
					_udpDest = IPinteger;
				}
					break;
				case 5: // Stop
				{
					Log::d("STOP");  // close the socket connection with the mobile APP
					_udpDest = (uint32_t) 0;
				}
					break;
				case 6: // Setted
				{
					// Reply
					_pktBuffer[46] = '\0';
					char *argument = (char *) _pktBuffer + 16;

					Config::setLongitude(atof(argument));

					argument = (char *) _pktBuffer + 26;
					Config::setLatitude(atof(argument));

					Log::d("Location received - latitude: %lf - longitude: %lf", Config::getLatitude(),
						   Config::getLongitude());

					_pktBuffer[5] = 6;
					_cmdc.beginPacket(_udpTemp, 62001);
					_cmdc.write(_pktBuffer, CONTROLPKTSIZE);// send ack lat/lon received
					_cmdc.endPacket();

					storeConfigToSD();
					start = true;
					memset(_pktBuffer, 0, 48);
				}
					break;
				default:
				{
					Log::e("Unknown command");
				}
					break;
			}
		} else {
			Log::e("Invalid packet magic");
		}
	}
}

// send the accelerometer values to the mobile APP
// TODO: check
void sendValues(struct RECORD *db) {
	uint32_t valx = (uint32_t)db->valx;
	uint32_t valy = (uint32_t)db->valy;
	uint32_t valz = (uint32_t)db->valz;
	if (_udpDest != 0) {  // if a socket connection with the mobile APP has been established
		_cmdc.beginPacket(_udpDest, 62002);
		ArrayToInteger cv;
		cv.integer = valx;
		_pktBuffer[0] = cv.array[0];
		_pktBuffer[1] = cv.array[1];
		_pktBuffer[2] = cv.array[2];
		_pktBuffer[3] = cv.array[3];

		cv.integer = valy;
		_pktBuffer[4] = cv.array[0];
		_pktBuffer[5] = cv.array[1];
		_pktBuffer[6] = cv.array[2];
		_pktBuffer[7] = cv.array[3];

		cv.integer = valz;
		_pktBuffer[8] = cv.array[0];
		_pktBuffer[9] = cv.array[1];
		_pktBuffer[10] = cv.array[2];
		_pktBuffer[11] = cv.array[3];

		_cmdc.write(_pktBuffer, 12);
		_cmdc.endPacket();
	}
}

// establish the connection through the 62001 port to interact with the mobile APP
void commandInterfaceInit() {
	_cmdc.begin(62001);
}
