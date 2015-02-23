#ifndef COMMONS_H_
#define COMMONS_H_
char* itoa(int num, char* str, int base);
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
static char *download_scriptText = "curl -o /media/realroot/sketch.elf  %s";
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
  if (path == NULL){// DOWNLOADED CREATION SCRIPT
    script = fopen(script_path, "w");
  }else {
    script = fopen(path, "w");
  }
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




/* ################################################################################################## */

char *floatToString(char * outstr, float value, int places, int minwidth=0, boolean rightjustify=false) {
  // this is used to write a float value to string, outstr.  oustr is also the return value.
  int digit;
  float tens = 0.1;
  int tenscount = 0;
  int i;
  float tempfloat = value;
  int c = 0;

  int charcount = 1;
  int extra = 0;
  char countdown[10];
  // make sure we round properly. this could use pow from <math.h>, but doesn't seem worth the import
  // if this rounding step isn't here, the value  54.321 prints as 54.3209

  // calculate rounding term d:   0.5/pow(10,places)  
  float d = 0.5;
  if (value < 0) d *= -1.0;
  // divide by ten for each decimal place
  for (i = 0; i < places; i++)
    d/= 10.0;    
  // this small addition, combined with truncation will round our values properly 
  tempfloat +=  d;

  // first get value tens to be the large power of ten less than value    
  if (value < 0) tempfloat *= -1.0;
  while ((tens * 10.0) <= tempfloat) {
    tens *= 10.0;
    tenscount += 1;
  }

  if (tenscount > 0) charcount += tenscount;
  else charcount += 1;

  if (value < 0)
    charcount += 1;
  charcount += 1 + places;

  minwidth += 1; // both count the null final character
  if (minwidth > charcount){        
    extra = minwidth - charcount;
    charcount = minwidth;
  }

  if (extra > 0 && rightjustify) {
    for (int i = 0; i< extra; i++) {
      outstr[c++] = ' ';
    }
  }

  // write out the negative if needed
  if (value < 0) outstr[c++] = '-';

  if (tenscount == 0) outstr[c++] = '0';

  for (i=0; i< tenscount; i++) {
    digit = (int) (tempfloat/tens);
    itoa(digit, &outstr[c++], 10);
    tempfloat = tempfloat - ((float)digit * tens);
    tens /= 10.0;
  }
  // if no places after decimal, stop now and return

  // otherwise, write the point and continue on
  if (places > 0)outstr[c++] = '.';
  // now write out each decimal place by shifting digits one by one into the ones place and writing the truncated value
  for (i = 0; i < places; i++) {
    tempfloat *= 10.0; 
    digit = (int) tempfloat;
    itoa(digit, &outstr[c++], 10);
    // once written, subtract off that digit
    tempfloat = tempfloat - (float) digit; 
  }
  if (extra > 0 && !rightjustify) {
    for (int i = 0; i< extra; i++) {
      outstr[c++] = ' ';
    }
  }
  outstr[c++] = '\0';
  return outstr;
}
/* ################################################################################################## */

float stringToFloat(char *buf){
  int i, temp;
  float value;
  i = strlen(buf) - 1;
  value = 0.0;
  while(i >= 0){
    switch(i){
        case 4: temp = buf[i] - '0';
                value += (temp / 100.0);
                break;
        
        case 3: temp = buf[i] - '0';
                value += (temp / 10.0);
                break;
                
        case 1: temp = buf[i] - '0';
                value += (temp * 1.0);
                break;
                
        case 0: temp = buf[i] - '0';
                value += (temp * 10.0);
                break;
    }
    i--;
  }
  return value;
}

/* ################################################################################################## */






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