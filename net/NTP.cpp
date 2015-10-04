
#include <stdio.h>
#include <string.h>
#include "NTP.h"
#include "../common.h"
#include "../Log.h"
#include "../Utils.h"

// NTP time stamp is in the first 48 bytes of the message
#define NTP_PACKET_SIZE 48
#define JAN_1970        2208988800L

time_t NTP::unixTimeTS = 0;
unsigned long NTP::unixTimeUpdate = 0;

IPaddr NTP::ntpserver(88, 149, 128, 123);
Udp NTP::udpSocket;

// send an NTP request to the time server at the given address
bool NTP::sendNTPpacket(IPaddr address) {
	byte packetBuffer[NTP_PACKET_SIZE];

	// set all bytes in the buffer to 0
	memset(packetBuffer, 0, NTP_PACKET_SIZE);

	// Initialize values needed to form NTP request
	// (see URL above for details on the packets)
	packetBuffer[0] = 0b11100011;   // LI, Version, Mode
	packetBuffer[1] = 0;     // Stratum, or type of clock
	packetBuffer[2] = 6;     // Polling Interval
	packetBuffer[3] = 0xEC;  // Peer Clock Precision

	// 8 bytes of zero for Root Delay & Root Dispersion
	packetBuffer[12] = 49;
	packetBuffer[13] = 0x4E;
	packetBuffer[14] = 49;
	packetBuffer[15] = 52;

	// all NTP fields have been given values, now
	// you can send a packet requesting a timestamp:
	ssize_t s = NTP::udpSocket.send(packetBuffer, (size_t)NTP_PACKET_SIZE, address, 123);
	return s > 0;
}

// Set date and time to NTP's retrieved one
void NTP::execSystemTimeUpdate(time_t epoch) {
	char command[100];
	snprintf(command, 100, SETDATE_CMD, epoch);

	char buf[64];
	FILE *ptr;

	Log::d("COMANDO: %s", command);
	if ((ptr = popen(command, "r")) != NULL) {
		while (fgets(buf, 64, ptr) != NULL) {
			std::string sbuf = std::string(buf);
			std::string nbuf = Utils::trim(sbuf, '\r');
			std::string vbuf = Utils::trim(nbuf, '\n');
			Log::d(Utils::trim(vbuf, ' ').c_str());
		}
		pclose(ptr);
	} else {
		Log::d("error popen NTP init");
	}
}

unsigned long NTP::getUNIXTime() {
	unsigned long diff = Utils::millis() - NTP::unixTimeUpdate;
	return (NTP::unixTimeTS + (diff / 1000));
}

unsigned long int NTP::getUNIXTimeMS() {
	unsigned long diff = Utils::millis() - NTP::unixTimeUpdate;
	return (((NTP::unixTimeTS) + (diff / 1000) + 1));
}

void NTP::setNTPServer(IPaddr ntpserver) {
	NTP::ntpserver = ntpserver;
}

void NTP::init() {
	udpSocket.listen(63451);
	udpSocket.setNonblocking();
}

bool NTP::sync() {
	Log::d("NTP sync with %s", NTP::ntpserver.asString().c_str());
	bool ret = false;
	if(NTP::ntpserver == 0) return false;
	// If current time is lower than ~ "2015-07-23 12:45:00" then we force NTP sync
	//
	// This function will loop when no NTP sync occurs and:
	//
	// - We are running on a device without clock info (eg. hardware clock) so clock is not coherent
	// - or, this device has traveled back in time
	//
	// Let's hope for first condition :-)
	do {
		// send an NTP packet to a time server
		NTP::sendNTPpacket(NTP::ntpserver);

		// wait to see if a reply is available
		Utils::delay(500);
		unsigned long responseMill = Utils::millis();

		// WAIT FOR SERVER RESPONSE
		while (!ret && Utils::millis() - responseMill < NTP_RESPONSE_TIMEOUT_VALUE) {
			byte packetBuffer[NTP_PACKET_SIZE];
			ssize_t r = NTP::udpSocket.receive(packetBuffer, (size_t)NTP_PACKET_SIZE, NULL, NULL);

			if (r >= NTP_PACKET_SIZE) {
				uint64_t ntptime = Utils::hton64(packetBuffer+40);
				time_t epoch = ntptime - JAN_1970;

				NTP::unixTimeTS = epoch; //* 1000UL;
				NTP::unixTimeUpdate = Utils::millis();

				// print Unix time:
				Log::d("Unix time = %i", epoch);
				Utils::delay(50);
				NTP::execSystemTimeUpdate(epoch);
				ret = true;
			}
		}
		if(!ret) {
			Log::e("ERROR NTP PACKET NOT RECEIVED: %s", NTP::ntpserver.asString().c_str());
		}
	} while(NTP::unixTimeTS < 1437648361);
	return ret;
}

int NTP::getHour() {
	time_t rawtime;
	time(&rawtime);
	struct tm *tm_struct = localtime(&rawtime);
	return tm_struct->tm_hour;
}

IPaddr NTP::getNTPServer() {
	return ntpserver;
}
