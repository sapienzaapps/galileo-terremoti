#ifndef config_h
#define config_h

#include "GalileoLog.h"

#define DEFAULT_HTTP_SERVER "www.sapienzaapps.it"
char* path_domain = "/terremoti/galileo";

byte mac[] = { 0x00, 0x13, 0x20, 0xFF, 0x13, 0x0d };  // fictitious MAC address
//byte mac[] = { 0x98, 0x4F, 0xEE, 0x00, 0x52, 0x18 };  // real MAC address
int deviceLocation = 2;  // 0 is for Sapienza Colossus | 1 is for Panizzi's room | 2 is for Home
boolean isDhcpEnabled = false;

bool debugON = true;  // is debugging mode enabled?
bool logON = true;  // is logging mode enabled?
bool ledON = false;  // are the leds mounted on the board?
bool deviceHasSDCard = false;  // is the SD card mounted on the board?

long pingIntervalCheck = 30 * 1000;
long pingIntervalCheckCounter = 0;
bool isConnected;

IPAddress ip;
IPAddress dns;
IPAddress gateway;
IPAddress subnet;

// ******** FINE CONFIGURAZIONE

char* httpServer;
IPAddress timeServer;

// struct for time and axis variations logging 
struct RECORD {
  unsigned long ts;
  long ms;
  long valx;
  long valy;
  long valz;
  boolean overThreshold;
};

struct TDEF {
  double pthresx;
  double pthresy;
  double pthresz;
  double nthresx;
  double nthresy;
  double nthresz;
};

//printing a record state
void printRecord(struct RECORD *db) {
	if (debugON) {
		Serial.print(db->valx);
		Serial.print("-");
		Serial.print(db->valy);
		Serial.print("-");
		Serial.print(db->valz);
	}
}

void forceConfigUpdate();

bool isConnectedToInternet() {
	int ping = system("bin/busybox ping -w 2 8.8.8.8");

	int pingWifexited = WIFEXITED(ping);
	if (pingWifexited) {
		if (WEXITSTATUS(ping) == 0) {
			isConnected = true;
			return true;
		}

		if (debugON) {
			Serial.print("Ping WEXITSTATUS STATUS: ");
			Serial.println(WEXITSTATUS(ping));
		}
	}
	else {
		if (debugON) {
			Serial.print("Ping Wifexited STATUS: ");
			Serial.println(pingWifexited);
		}
		isConnected = false;
		return false;
	}

	isConnected = false;
	return false;
}

#endif
