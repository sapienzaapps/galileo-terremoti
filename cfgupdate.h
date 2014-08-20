#ifndef cfgupdate_h
#define cfgupdate_h 1

unsigned long lastCfgUpdate = 0;
unsigned long cfgUpdateInterval = 60;

IPAddress getFromString(char* ipAddr) {
  char *p1 = strchr(ipAddr, '.');
  char* p2 = strchr(p1+1, '.');
  char* p3 = strchr(p2+1, '.');
  
  *p1 = 0;
  *p2 = 0;
  *p3 = 0;
  
  uint32_t d1 = atoi(ipAddr);
  uint32_t d2 = atoi(p1+1);
  uint32_t d3 = atoi(p2+1);
  uint32_t d4 = atoi(p3+1);
  
  return (uint32_t)(d1*16777216 + d2*65536 + d3*256 + d4);
}

boolean getConfigUpdates(boolean noupdate) {
  boolean ret = false;
  if(client.connect(httpServer, 80)) {
    Serial.print("Requesting CONFIG to: ");
    Serial.println(httpServer);
    
    client.print("GET ");
    client.print(path_domain);
    client.print("/device.php?op=config&mac=");
    for(int m=0; m < 6; m++) {
      if(mac[m] < 0x10) client.print("0");
      client.print(mac[m], HEX);
    }
    client.println(" HTTP/1.1");
    client.print("Host: ");
    client.println(httpServer);
    client.println("Connection: close");
    client.println("");
    
    char rBuffer[300];
    // Reading headers
    int s = getLine(client, rBuffer, 300);

    if(strncmp(rBuffer, "HTTP/1.1 200", 12) == 0) {  // if it is an HTTP 200 response
      int bodySize = 0;
      do {
        s = getLine(client, rBuffer, 300);
        if(strncmp(rBuffer, "Content-Length", 14) == 0) {
          char* separator = strchr(rBuffer, ':');
          if(*(separator+1) == ' ') {
            separator += 2;
          } else {
            separator++;
          }
          bodySize = atoi(separator);
        }
      } while(s > 0);
      // Content
      
      do {
        s = getLine(client, rBuffer, 300);
        if(s > 0) {
          char* separator = strchr(rBuffer, ':');
          *separator = 0;
          char* argument = separator+1;
          if(strncmp(rBuffer, "server", 6) == 0) {
            free(httpServer);
            httpServer = (char*)malloc(strlen(argument)*sizeof(char));
            strcpy(httpServer, argument);
          } else if(strncmp(rBuffer, "ntpserver", 9) == 0) {
            timeServer = getFromString(argument);
          }
        }
      } while(s > 0);
      ret = true;
    } else {
      Serial.print("Error in reply: ");
      Serial.println(rBuffer);
    }
    client.stop();
  }
  if(!noupdate) lastCfgUpdate = getUNIXTime();
  return ret;
}


void doConfigUpdates() {
  if(lastCfgUpdate+cfgUpdateInterval < getUNIXTime() && isConnectedToInternet()) {

    // Get Updates
    if(getConfigUpdates(false)) {
      Serial.println("Configuration update succeded");
      /*
      Serial.print("HTTP Server: ");
      Serial.println(httpServer);
      Serial.print("NTP Server (cfgupdate): ");
      Serial.println(timeServer);
      */
    } else {
      Serial.println("Configuration update failed");
    }
  }
}

void forceConfigUpdate(boolean noupdate) { //controllare frequenza chiamata
  boolean ret = getConfigUpdates(noupdate);
  while(!ret) { 
    Serial.println("Configuration update failed, retrying in 3 seconds...");
    delay(3000);
    ret = getConfigUpdates(noupdate);
  }
}

void forceConfigUpdate() {
  return forceConfigUpdate(false);  // set to FALSE to force the update of the last configuration time (for the NTP sync)
}

// get the HTTP Server and NTP Server
void initConfigUpdates() {
  httpServer = (char*)malloc(strlen(DEFAULT_HTTP_SERVER) * sizeof(char));
  strcpy(httpServer, DEFAULT_HTTP_SERVER);
  
  forceConfigUpdate();
  /*
  Serial.print("HTTP Server: ");
  Serial.println(httpServer);
  Serial.print("NTP Server (init cfupdate): ");
  Serial.println(timeServer);
  */
}

#endif
