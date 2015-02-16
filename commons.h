#ifndef COMMONS_H_
#define COMMONS_H_

IPAddress ip;
IPAddress dns;
IPAddress gateway;
IPAddress subnet;

char* httpServer;
IPAddress timeServer;

FILE *script;
/* static char *script_path = "/gscript/prova.sh";
static char *script_reset =  "/gscript/reset.sh"; */
static char *script_path = "media/realroot/prova.sh";
static char *script_reset =  "media/realroot/reset.sh";
static char *reboot_scriptText = "#!/bin/bash\nshutdown -r -t sec 00\n";
unsigned long resetConnetcionMills = 0;
unsigned long resetConnectionInterval = 3*1000;

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
// check if Internet Connection is available
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


// check if a file exists
int doesFileExist(const char *filename) {
	if( access( filename, F_OK ) != -1 ) {
	  // file exists
		return 1;
	}
	else {
	  // file doesn't exist
		return 0;
	}
}


// create a script
void createScript(char *path, char *text) {
	script = fopen(path, "w");
	if (script  == NULL) {
    if (debugON) Serial.println("F_Error opening script!\n");
	  //exit(1);
    return;
	}else{
    if (debugON) Serial.println(text);
    //fprintf(script, "%s> ", getGalileoDate());
    fprintf(script, "%s\n", text);
    fclose(script);
    delay(5);
    char *rights = "chmod a+rx %s";
    char str[80];
    int len = sprintf(str, rights, path);
    if (false){// debug only
      Serial.print("createScript - bytes written: ");
      Serial.println(len);
    }
    //str[len] = '\0';// togliere!!!??
    //system("chmod a+rx /gscript/prova.sh");
    system(str);
    Serial.println("chmod a+rx script file");
  }
}

// execute a script
void execScript(char *path) {
        if (doesFileExist(path)){
          if (debugON){
            Serial.print("executing script: ");
            Serial.println(path);
          }
          if (logON) log(path);
          delay(5);
          system(path);
          //system("/gscript/prova.sh");
          delay(500);
        }else{
          if (debugON) Serial.println("script not found!!!");
          if (logON) log("script not found!!!");
        }
}

// RESET CONNECTION IF IS NOT CONNECTED TO INTERNET
void resetConnection(int numTry){
  if (millis() - resetConnetcionMills > resetConnectionInterval){
    resetConnetcionMills = millis();
    if (debugON){
      Serial.print("Trying to restore INTERNET CONNECTION: ");
      Serial.println(numTry);
    }
    if (logON){
      log("Trying to restore INTERNET CONNECTION: ");
      logInt(numTry);
    }
    if(numTry % 2 == 0){  // LED RESET CONNECTION BLINK
      digitalWrite(10, LOW);
      digitalWrite(12, HIGH);
    
    }else{
      digitalWrite(10, HIGH);
      digitalWrite(12, LOW);
    }
    // Workaround for Galileo (and other boards with Linux)
    system("/etc/init.d/networking restart");
    delay(3000);
    internetConnected = isConnectedToInternet();
    if (!internetConnected){// TEST TEST TEST TEST!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
      if (debugON){ 
        Serial.println("---- FAILED ----  restore Internet connection");
        Serial.println("+++++ setupEthernet() ++++++");
      }
      if (logON){ 
        log("---- FAILED ----  restore Internet connection");
        log("+++++ setupEthernet() ++++++");
      }
      //setupEthernet();// After network restart try to set Arduino network
      delay(1000);
      if(!(internetConnected = isConnectedToInternet()) && numTry <= 0){
        // TRYING TO REBOOT DEVICE
        if (debugON) Serial.println("----- REBOOT GALILEO -----");
        if (logON) log("----- REBOOT GALILEO -----");
        
        //system("reboot");
        if(!doesFileExist(script_reset)){ // check if reboot script exists
          createScript(script_path, reboot_scriptText);
          delay(5);          
        }
        //system("/gscript/reset.sh");
        system(script_reset);
        while(1){;} // lock HERE for  SYSTEM RESET
      }
    }else {
      if (debugON) Serial.println("---- SUCCESS ----  restore Internet connection");
      if (logON) log("---- SUCCESS ----  restore Internet connection");
    }
  }
}

//avr-objdump -S {compiled *.elf file}
/* COMMONS_H_ */



/* void galileoCreateFile(String fileName) {
Serial.println("\n*****Creation Started*****");
String status_message = String();
status_message = fileName;
char charFileName[fileName.length() + 1];
fileName.toCharArray(charFileName, sizeof(charFileName));
 
if (SD.exists(charFileName)) {
status_message += " exists already.";
}
else {
char system_message[256];
char directory[] = "/media/realroot";
sprintf(system_message, "touch %s/%s", directory, charFileName);
system(system_message);
if (SD.exists(charFileName)) {
status_message += " created.";
}
else {
status_message += " creation tried and failed.";
}
}
Serial.println(status_message);
}  */
#endif 