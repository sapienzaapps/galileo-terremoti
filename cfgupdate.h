#ifndef cfgupdate_h
#define cfgupdate_h 1

#include "GalileoLog.h"
FILE *fp;
unsigned long lastCfgUpdate = 0;
unsigned long cfgUpdateInterval = 60;

long previousMillisConfig = 0;        // will store last time LED was updated
long intervalConfig = 1*60*1000;// 3 minuti

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

// prepare buffer for config update request
int prepareConfigBuffer(char* buf) {
	// deviceid, lat, lon, pthresy, version, model
   if(debugON) Serial.print("mac testo: ");
   if(debugON) Serial.println(mac_string);
  return sprintf(buf, "deviceid=%s&lat=%s&lon=%s&version=%.2f&model=%s", mac_string, configGal.lat,configGal.lon, configGal.version, configGal.model );
}

/* boolean getConfigUpdates(boolean noupdate) {
  boolean ret = false;
  if (client.connect(httpServer, 80)) {
  	if (debugON) Serial.print("Requesting CONFIG to: ");
  	if (debugON) Serial.println(httpServer);
    
    client.print("GET ");
    client.print(path_domain);
    client.print("/device.php?op=config&mac=");
    for (int m=0; m < 6; m++) {
      if (mac[m] < 0x10) client.print("0");
      client.print(mac[m], HEX);
    }
    client.println(" HTTP/1.1");
    client.print("Host: ");
    client.println(httpServer);
    client.println("Connection: close");
    client.println("");
    
    delay(100); // ATTENDERE ARRIVO RISPOSTA!!!
    while (!client.available()) {;}  // Attendere che il client risponda
    Serial.println("Dati arrivati - getConfigUpdates");
    char rBuffer[300];
    // Reading headers
    int s = getLine(client, rBuffer, 300);

    if (strncmp(rBuffer, "HTTP/1.1 200", 12) == 0) {  // if it is an HTTP 200 response
      int bodySize = 0;
      do {
        s = getLine(client, rBuffer, 300);
        if (strncmp(rBuffer, "Content-Length", 14) == 0) {
          char* separator = strchr(rBuffer, ':');
          if (*(separator+1) == ' ') {
            separator += 2;
          }
          else {
            separator++;
          }
          bodySize = atoi(separator);
        }
      } while(s > 0);
      // Content
      
      do {
        s = getLine(client, rBuffer, 300);
        if (s > 0) {
          char* separator = strchr(rBuffer, ':');
          *separator = 0;
          char* argument = separator+1;
          if (strncmp(rBuffer, "server", 6) == 0) {
            free(httpServer);  // ?
            httpServer = (char*)malloc(strlen(argument)*sizeof(char));
            if(httpServer!=NULL){
              strcpy(httpServer, argument);
            }else{
              if (logON) log("Malloc FAILED - getConfigUpdates");
              if (debugON) Serial.println("Malloc FAILED - getConfigUpdates");
            }
          }
          else if(strncmp(rBuffer, "ntpserver", 9) == 0) {
            timeServer = getFromString(argument);
          }
        }
      } while(s > 0);
      ret = true;
    }
    else {
    	if (debugON) Serial.print("Error in reply: ");
    	if (debugON) Serial.println(rBuffer);
    }
    client.stop();
  }else{
      client.stop();
      if(debugON) Serial.println("Connection error");
      if(logON)log("connessione fallita");
      resetEthernet = true;
  }
  if (!noupdate) lastCfgUpdate = getUNIXTime();
  return ret;
} */

/* void doConfigUpdates() {
	//unsigned long currentMillisConfig = millis();
	//if (currentMillisConfig - previousMillisConfig > intervalConfig) {
		//previousMillisConfig = currentMillisConfig;
		log("Still running Config Update");
		//if (isConnectedToInternet()) {
    log("isConnectedToInternet");
		//}
		log("lastCfgUpdate");
		logLong(lastCfgUpdate);
		log("cfgUpdateInterval");
		logLong(cfgUpdateInterval);
		log("getUNIXTime()");
		logLong(getUNIXTime());
	//}

  //if (lastCfgUpdate+cfgUpdateInterval < getUNIXTime() && isConnectedToInternet()) {
    // Get Updates
    if (getConfigNew(false)) {
    	if (debugON) Serial.println("Configuration update succeded");
    	if (logON) log("Configuration update succeded");
    }
    else {
    	if (debugON) Serial.println("Configuration update failed");
    	if (logON) log("Configuration update failed");
    }
 // }
}



/* void forceConfigUpdate() {
  return forceConfigUpdate(false);  // set to FALSE to force the update of the last configuration time (for the NTP sync)
} */



// ask config to server - New Da finire
boolean getConfigNew() {
  Serial.println("getConfigNew()------------------ START ----------------------");
  //inEvent = 1;
  //milldelayTimeEvent = millis(); // timestamp in millis for Event Interval
  boolean ret = false;
  if (client.connect(httpServer, 80)) {
  	if (debugON) Serial.print("Requesting CONFIG to: ");
  	if (debugON) Serial.println(httpServer);
    char rBuffer[300];
    int rsize = prepareConfigBuffer(rBuffer);  // prepare the info for the new entry to be send to DB
    // sendig request to server
    client.print("POST ");
    client.print(path_domain);
    client.print("/alive.php");
    client.println(" HTTP/1.1");
    client.print("Host: ");
    client.println(httpServer);
    client.println("Content-Type: application/x-www-form-urlencoded");
    client.print("Content-Length: ");
    client.println(rsize);
    client.println("Connection: close"); // ???
    client.println("");
    client.println(rBuffer);
    //client.println("");
    if(debugON) Serial.print("sending Buffer: "); 
    if(debugON) Serial.println(rBuffer); 
    delay(100); // ATTENDERE ARRIVO RISPOSTA!!!
    unsigned long responseMill = millis();
    // while (!client.available()) {;}  // Attendere che il client risponda
    while(!client.available() && (millis() - responseMill < timeoutResponse ) ){;}
    if (millis() - responseMill > timeoutResponse){ 
      if(debugON)Serial.println("TIMEOUT SERVER CONNECTION");
      if(logON)log("TIMEOUT SERVER CONNECTION- getConfigNew()");
    }else{ // data arrived
      if(debugON)Serial.println("Dati arrivati - getConfigNew()");
      memset( rBuffer, 0, 300*sizeof(char));
      // Reading headers
      int s = getLine(client, rBuffer, 300);

      if (s & strncmp(rBuffer, "HTTP/1.1 200", 12) == 0) {  // if it is an HTTP 200 response
        int bodySize = 0;
        do {
          s = getLine(client, rBuffer, 300);
          if (strncmp(rBuffer, "Content-Length", 14) == 0) {
            char* separator = strchr(rBuffer, ':');
            if (*(separator+1) == ' ') {
              separator += 2;
            }
            else {
              separator++;
            }
            bodySize = atoi(separator);
          }
        } while(s > 0);
        // Content after "\n" , there is body response --------------------
        int p = 0;
        bool exec_status = false;
        memset( rBuffer, 0, 300*sizeof(char));
        do {
          // Serial.println("GetPipe");
          p = getPipe(client, rBuffer, 300);
          if (p > 0) {
            size_t l;
            char* separator = strchr(rBuffer, ':');
            *separator = 0;
            char* argument = separator+1;
            l = strlen(argument);
            if (strncmp(rBuffer, "server", 6) == 0) { // SERVER TO CONTACT 
              free(httpServer);  // ?
              httpServer = (char*)malloc(l*sizeof(char));
             
              Serial.print("dimensione server: ");
              Serial.println(l, DEC);
              Serial.print("Argomento:#");
              Serial.print(argument);
              Serial.println("#");
              if(httpServer!=NULL && (l > 0) ){
                strncpy(httpServer, argument,l);
                httpServer[l] = '\0';
                Serial.print("Server: ");
                Serial.println(httpServer);
                if (logON){
                  log("cfg Server:");
                  log(httpServer);
                }  
              }else{
                if (logON) log("Malloc FAILED - getConfigUpdates");
                if (debugON) Serial.println("Malloc FAILED - getConfigUpdates");
              }
            }
            else if(strncmp(rBuffer, "ntpserver", 9) == 0) { // Ntpserver 
              // char* separator = strchr(rBuffer, ':');
              // *separator = 0;
              // char* argument = separator+1;
              Serial.println(argument);
              timeServer = getFromString(argument);
            }
            else if(strncmp(rBuffer, "script", 6) == 0) { // Check for executing script
              if (strlen(argument) > 0){
                char scriptTest[100] ;
                size_t len = strlen(argument);
                strncpy(scriptTest, argument, len);
                scriptTest[len] = '\0';
                createScript("/media/realroot/script.sh", scriptTest);
                exec_status = true;
                if(debugON) Serial.println("Script Creation...");
              }
              if(debugON){
                Serial.print("Script length: ");
                Serial.println(strlen(argument), DEC);
                Serial.print("Script: ");
                Serial.println(argument);
              }
            }
            else if(strncmp(rBuffer, "path", 4) == 0) { // Check for downloading file
              size_t len = strlen(argument);
              if (len > 0){
                char pathTest[300] ;
                char pathScriptDownload[300] ;
                strncpy(pathTest, argument, len); // remote peth for file downloading
                pathTest[len] = '\0';
                sprintf(pathScriptDownload,download_scriptText,pathTest); 
                Serial.print("pathTest: ");
                Serial.println(pathTest);
                Serial.print("pathScriptDownload: ");
                Serial.println(pathScriptDownload);
                createScript(NULL, pathScriptDownload); // creation script for download a file from the path(internet resource) 
                if(debugON){
                  Serial.print("execScript for Download....");
                }
                execScript(script_path); // executing download of the file
                delay(1000);
                // system("cp /media/mmcblk0p1/sketch/sketch.elf /sketch/sketch.elf");
                // system("chmod a+rx /sketch/sketch.elf");
              }
              if(debugON){
                Serial.print("Path length: ");
                Serial.println(strlen(argument), DEC);
                Serial.print("path: ");
                Serial.println(argument);
              }
            }
          }
            // char* separator = strchr(rBuffer, ':');
            // *separator = 0;
            // char* argument = separator+1;
        } while(p > 0);
        if (exec_status){ // check for executing script command
          execScript("/media/realroot/script.sh");
          if(debugON){
            Serial.print("execScript....");
            Serial.print("/media/realroot/script.sh");
          }
          for(int x = 0; x < 3; x++){
            resetBlink(0);
          }
        }
        ret = true;
      }
      else { // not 200 response
        if (debugON) Serial.print("Error in reply: ");
        if (debugON) Serial.println(rBuffer);
      }
    } // end data arrived
    //client.stop();
  }else{ // impossible to contact the server
      client.stop();
      errors_connection++;
      if(debugON) Serial.println("Connection error");
      if(logON)log("getConfigNew() - connessione fallita");
      //resetEthernet = true;
  }
  
  
  while (client.connected()) {
    Serial.println();
    Serial.println("disconnecting.");
    client.stop();
  }
  
  if(logON){
    log("Still running Config Update");
    if (isConnectedToInternet()) {
      log("isConnectedToInternet");
    }
    log("lastCfgUpdate");
    logLong(lastCfgUpdate);
    log("cfgUpdateInterval");
    logLong(cfgUpdateInterval);
    log("getUNIXTime()");
    logLong(getUNIXTime());
    if(!ret) log("getConfigNew() Update ERROR!!!");
  }
  Serial.println("getConfigNew()------------------ EXIT ----------------------");
  return ret;
}


/* void forceConfigUpdate(boolean noupdate) { //controllare frequenza chiamata
  //boolean ret = getConfigUpdates(noupdate);
  boolean ret = getConfigNew();
  while(!ret) { 
  	if (debugON) Serial.println("Configuration update failed, retrying in 3 seconds...");
    delay(3000);
    //ret = getConfigUpdates(noupdate);
    ret = getConfigNew();
  }
} */

// get the HTTP Server(default if not) and NTP Server
void initConfigUpdates() {
  httpServer = (char*)malloc(strlen(DEFAULT_HTTP_SERVER) * sizeof(char));
  if(httpServer != NULL){
    strcpy(httpServer, DEFAULT_HTTP_SERVER);
  }else{
    if (logON) log("Malloc FAILED - getConfigUpdates");
    if (debugON) Serial.println("Malloc FAILED - getConfigUpdates");
  }
 // Read log.txt size, if too big delete it
/*  FILE *fp2 = fopen("/media/realroot/log.txt", "a");
 fseek(fp2, 0L, SEEK_END);
 int sz = ftell(fp2);
 fclose (fp2);
 if (sz > 10000) {
   system("rm /media/realroot/log.txt"); // remove logfile if too old 
   if(debugON) Serial.println("log file removed");
   if(logON) log("log file removed");
 } */
  if (internetConnected && start ){ // get config onli if Galileo is connected and lat/lon are setted
    //forceConfigUpdate(false);
    boolean ret = getConfigNew();
    int nTimes = 0;
    while(!ret && (nTimes < 5)) { 
      nTimes++;
      if (debugON) Serial.println("Configuration update failed, retrying in 3 seconds...");
      delay(3000);
      //ret = getConfigUpdates(noupdate);
      ret = getConfigNew();
    }
    if(nTimes >=5) Serial.println("getConfigNew()  -  failed!!!!!");
  }
}



#endif
