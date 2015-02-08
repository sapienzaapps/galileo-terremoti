#ifndef COMMONS_H_
#define COMMONS_H_

IPAddress ip;
IPAddress dns;
IPAddress gateway;
IPAddress subnet;

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

static struct RECORD recddl = {0, 0, 0, 0, 0, false};
struct RECORD *rec = &recddl;

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

	if ((deviceLocation == 0) || (!testNoInternet)) {
		return true;
	}

	int pingWifexited = WIFEXITED(ping);
	if (pingWifexited) {
		if (WEXITSTATUS(ping) == 0) {
			internetConnected = true;
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
		internetConnected = false;
		return false;
	}

	internetConnected = false;
	return false;
}


#endif /* COMMONS_H_ */
