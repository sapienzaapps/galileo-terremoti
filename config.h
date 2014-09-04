#ifndef config_h
#define config_h

#include "GalileoLog.h"

#define DEFAULT_HTTP_SERVER "www.sapienzaapps.it"
char* path_domain = "/terremoti/galileo";

byte mac[] = { 0x00, 0x13, 0x20, 0xFF, 0x13, 0x0c };  // fictitious MAC address
int deviceLocation = 2;  // 0 is for Sapienza Colossus | 1 is for Panizzi's room | 2 is for Home
boolean isDhcpEnabled = false;

bool debugON = true;
bool logON = true;
long n_NTP_connections = 0;  // for debug purpose only

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

	//Serial.print("Ping STATUS: ");
	//Serial.println(ping);
	int pingWifexited = WIFEXITED(ping);
	//Serial.print("Ping Wifexited STATUS: ");
	//Serial.println(pingWifexited);
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

	/*
	if (ping == 0) {
		//Serial.println("*** CONNECTED ***");
		isConnected = true;
		return true;
	}
	else {
		//Serial.println("*** DISCONNECTED ***");
		isConnected = false;
		return false;
	}
	*/
}

bool isConnectedToInternet2() {
	unsigned long currentMillis = millis();
	if(isConnected || currentMillis - pingIntervalCheckCounter > pingIntervalCheck) {
	    // save the last time you tried to ping
		pingIntervalCheckCounter = currentMillis;

		int a = system("bin/busybox ping -w 2 8.8.8.8");
		if (a == 0) {
			//Serial.println("*** CONNECTED ***");
			isConnected = true;
			return true;
		}
		else {
			//Serial.println("*** DISCONNECTED ***");
			isConnected = false;
			return false;
		}
	}
}

#endif
