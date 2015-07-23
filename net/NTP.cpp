#include <stdio.h>
#include <string.h>
#include <UtilTime.h>
#include "NTP.h"
#include "../common.h"
#include "../Config.h"
#include "../Log.h"
#include "NetworkManager.h"
#include "../Utils.h"

// NTP time stamp is in the first 48 bytes of the message
#define NTP_PACKET_SIZE 48

unsigned long NTP::unixTimeTS = 0;
unsigned long NTP::unixTimeUpdate = 0;

IPAddress NTP::ntpserver(88, 149, 128, 123);
EthernetUDP NTP::udpSocket;

// send an NTP request to the time server at the given address
void NTP::sendNTPpacket(IPAddress &address) {
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
	NTP::udpSocket.beginPacket(address, 123); //NTP requests are to port 123
	NTP::udpSocket.write(packetBuffer, NTP_PACKET_SIZE);
	NTP::udpSocket.endPacket();
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

void NTP::setNTPServer(IPAddress ntpserver) {
	NTP::ntpserver = ntpserver;
}

void NTP::initNTP() {
	udpSocket.begin(63554);
	delay(1000);
	if (NTP::sync()) {
		Log::d("NTP updated: %lu", NTP::getUNIXTime());
	} else {
		Log::d("Errore NTPdataPacket() ");
	}
}

bool NTP::sync() {
	bool ret = false;
	// If current time is lower than ~ "2015-07-23 12:45:00" then we force NTP sync
	//
	// This function will loop when no NTP sync occurs and:
	//
	// - We are running on a device without clock info (eg. hardware clock) so clock is not coherent
	// - or, this device has traveled back in time
	//
	// Let's hope for first condition :-)
	do {
		if (NetworkManager::isConnectedToInternet()) {
			// send an NTP packet to a time server
			NTP::sendNTPpacket(NTP::ntpserver);

			// wait to see if a reply is available
			delay(2000);
			unsigned long responseMill = millis();

			// WAIT FOR SERVER RESPONSE
			while (!ret && millis() - responseMill < NTP_RESPONSE_TIMEOUT_VALUE) {
				if (NTP::udpSocket.parsePacket() >= NTP_PACKET_SIZE) {
					byte packetBuffer[NTP_PACKET_SIZE];

					// We've received a packet, read the data from it
					NTP::udpSocket.read(packetBuffer, NTP_PACKET_SIZE);  // read the packet into the buffer

					//the timestamp starts at byte 40 of the received packet and is four bytes,
					// or two words, long. First, esxtract the two words:
					unsigned long highWord = Utils::fixword(packetBuffer[40], packetBuffer[41]);
					unsigned long lowWord = Utils::fixword(packetBuffer[42], packetBuffer[43]);

					// combine the four bytes (two words) into a long integer
					// this is NTP time (seconds since Jan 1 1900):
					unsigned long secsSince1900 = highWord << 16 | lowWord;
					Log::d("Seconds since Jan 1 1900 = %ld", secsSince1900);

					// now convert NTP time into everyday time:
					// Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
					const unsigned long seventyYears = 2208988800UL;

					// subtract seventy years:
					unsigned long epoch = secsSince1900 - seventyYears;

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
		} else {
			// Internet not connected while try to sync with NTP
			Log::e("ERROR INTERNET NOT PRESENT IN: NTPdataPacket()");
			if(NTP::unixTimeTS < 1437648361) {
				sleep(2000);
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
