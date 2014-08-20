#ifndef config_h
#define config_h

#define DEFAULT_HTTP_SERVER "www.sapienzaapps.it"
char* path_domain = "/terremoti/galileo";

byte mac[] = { 0x00, 0x13, 0x20, 0xFF, 0x13, 0x0c };  // fictitious MAC address
int deviceLocation = 2;  // 0 is for Sapienza Colossus | 1 is for Panizzi's room | 2 is for Home
boolean isDhcpEnabled = false;

long n_NTP_connections = 0;  // for debug purpose only

long pingIntervalCheck = 30 * 1000;
long pingIntervalCheckCounter = 0;
bool isConnectedToInternet;

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
  Serial.print(db->valx);
  Serial.print("-");
  Serial.print(db->valy);
  Serial.print("-");
  Serial.print(db->valz);
}

void forceConfigUpdate();

bool isConnectedToInternet() {
	int a = system("bin/busybox ping -w 2 8.8.8.8");
	if (a == 0) {
		//Serial.println("*** CONNECTED ***");
		isConnectedToInternet = true;
		return true;
	}
	else {
		//Serial.println("*** DISCONNECTED ***");
		isConnectedToInternet = false;
		return false;
	}
}

bool isConnectedToInternet2() {
	unsigned long currentMillis = millis();
	if(isConnectedToInternet || currentMillis - pingIntervalCheckCounter > pingIntervalCheck) {
	    // save the last time you tried to ping
		pingIntervalCheckCounter = currentMillis;

		int a = system("bin/busybox ping -w 2 8.8.8.8");
		if (a == 0) {
			//Serial.println("*** CONNECTED ***");
			isConnectedToInternet = true;
			return true;
		}
		else {
			//Serial.println("*** DISCONNECTED ***");
			isConnectedToInternet = false;
			return false;
		}
	}
}

#endif
