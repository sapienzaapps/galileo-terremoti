
#include <stdio.h>
#include <string.h>
#include "NTP.h"
#include "../common.h"
#include "../Log.h"
#include "../Utils.h"

// NTP time stamp is in the first 48 bytes of the message
#define NTP_PACKET_SIZE 48
#define JAN_1970        2208988800L

unsigned long NTP::unixTimeTS = 0;
unsigned long NTP::unixTimeUpdate = 0;

IPaddr NTP::ntpserver(88, 149, 128, 123);
Udp NTP::udpSocket;

// send an NTP request to the time server at the given address
void NTP::sendNTPpacket(IPaddr address) {
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
	NTP::udpSocket.send(packetBuffer, (size_t)NTP_PACKET_SIZE, address, 123);
}

// Set date and time to NTP's retrieved one
void NTP::execSystemTimeUpdate(unsigned long epoch) {
	char command[100];
	snprintf(command, 100, "/bin/date -s @%lu", epoch);

	char buf[64];
	FILE *ptr;

	Log::d("COMANDO: %s", command);
	if ((ptr = popen(command, "r")) != NULL) {
		while (fgets(buf, 64, ptr) != NULL) {
			Log::d(buf);
		}
		pclose(ptr);
	} else {
		Log::d("error popen NTP init");
	}
}

unsigned long NTP::getUNIXTime() {
	unsigned long diff = millis() - NTP::unixTimeUpdate;
	return (NTP::unixTimeTS + (diff / 1000));
}

unsigned long int NTP::getUNIXTimeMS() {
	unsigned long diff = millis() - NTP::unixTimeUpdate;
	return (((NTP::unixTimeTS) + (diff /= 1000) + (diff % 1000 >= 0 ? 1 : 0)));
}

void NTP::setNTPServer(IPaddr ntpserver) {
	NTP::ntpserver = ntpserver;
}

//void NTP::initNTP() {
//	//udpSocket.begin(63554);
//	//delay(1000);
//	if (NTP::sync()) {
//		Log::d("NTP updated: %lu", NTP::getUNIXTime());
//	} else {
//		Log::d("Errore NTPdataPacket() ");
//	}
//}

bool NTP::sync() {
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
		delay(2000);
		unsigned long responseMill = millis();

		// WAIT FOR SERVER RESPONSE
		while (!ret && millis() - responseMill < NTP_RESPONSE_TIMEOUT_VALUE) {
			byte packetBuffer[NTP_PACKET_SIZE];
			int r = NTP::udpSocket.receive(packetBuffer, (size_t)NTP_PACKET_SIZE, NULL, NULL);

			if (r >= NTP_PACKET_SIZE) {
				uint64_t ntptime = Utils::hton64(packetBuffer+40);
				time_t epoch = ntptime - JAN_1970;

				NTP::unixTimeTS = (epoch); //* 1000UL;
				NTP::unixTimeUpdate = millis();

				// print Unix time:
				Log::d("Unix time = %i", epoch);
				delay(50);
				NTP::execSystemTimeUpdate(epoch);
				ret = true;
			} else {
				Log::e("ERROR NTP PACKET NOT RECEIVED");
			}
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
