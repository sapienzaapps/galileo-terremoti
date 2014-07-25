#ifndef config_h
#define config_h

#define DEFAULT_HTTP_SERVER "www.sapienzaapps.it"
char* path_domain = "/terremoti/galileo";

byte mac[] = { 0x00, 0x13, 0x20, 0xFF, 0x13, 0x0c };  // fictitious MAC address
int deviceLocation = 2;  // 0 is for Sapienza Colossus | 1 is for Panizzi's room | 2 is for Home
boolean isDhcpEnabled = false;

long n_NTP_connections = 0;  // for debug purpose only

IPAddress ip;
IPAddress dns;
IPAddress gateway;
IPAddress subnet;

// ******** FINE CONFIGURAZIONE

char* httpServer;
IPAddress timeServer;


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

void printRecord(struct RECORD *db) {
  Serial.print(db->valx);
  Serial.print("-");
  Serial.print(db->valy);
  Serial.print("-");
  Serial.print(db->valz);
}

void forceConfigUpdate();

#endif
