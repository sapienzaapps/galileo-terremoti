#ifndef COMMONS_H_
#define COMMONS_H_

//char* itoa(int num, char* str, int base);
IPAddress ip;
IPAddress dnsServer;
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
// static char *reboot_scriptText = "#!/bin/bash\nreboot\n";
static char *download_scriptText = "curl -o /media/realroot/sketch.elf  %s";
unsigned long resetConnetcionMills = 0;
unsigned long resetConnectionInterval = 3*1000;


// struct for time and axis variations logging
struct RECORD {
  unsigned long ts;
  unsigned long ms;
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
  Log::d("%ld:%ld:%ld", db->valx, db->valy, db->valz);
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
    Log::d("Ping WEXITSTATUS STATUS: %i", WEXITSTATUS(ping));
  }
  else {
    Log::d("Ping WEXITSTATUS STATUS: %i", pingWifexited);
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
void createScript(const char *path, char *text) {
  if (path == NULL){// DOWNLOADED CREATION SCRIPT
    script = fopen(script_path, "w");
  }else {
    script = fopen(path, "w");
  }
  if (script  == NULL) {
    Log::d("F_Error opening script!\n");
    //exit(1);
    return;
  }else{
    Log::d(text);
    //fprintf(script, "%s> ", getGalileoDate());
    fprintf(script, "%s\n", text);
    fclose(script);

    delay(5);

    char *rights = "chmod a+rx %s";
    char str[80];
    int len = sprintf(str, rights, path);

    Log::d("createScript - bytes written: %d", len);

    //str[len] = '\0';// togliere!!!??
    //system("chmod a+rx /gscript/prova.sh");

    system(str);
    Log::i("chmod a+rx script file");
  }
}

// execute a script
void execScript(const char *path) {
        if (doesFileExist(path)){
          Log::d("Executing script: %s", path);
          delay(5);
          system(path);
          delay(500);
        }else{
          Log::e("script not found!!!");
        }
}

// RESET CONNECTION IF IS NOT CONNECTED TO INTERNET
void resetConnection(int numTry){
  if (millis() - resetConnetcionMills > resetConnectionInterval){
    resetConnetcionMills = millis();
	Log::i("Trying to restore INTERNET CONNECTION: %i", numTry);
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
      Log::e("---- FAILED ----  restore Internet connection");
      Log::e("+++++ setupEthernet() ++++++");

      //setupEthernet();// After network restart try to set Arduino network
      delay(1000);
      if(!(internetConnected = isConnectedToInternet()) && numTry <= 0){
        // TRYING TO REBOOT DEVICE
        Log::e("----- REBOOT GALILEO -----");
        
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
      Log::i("---- SUCCESS ----  restore Internet connection");
    }
  }
}


//////////////////////////////////////////////////////////////////////////////
// Yet, another good itoa implementation
// returns: the length of the number string
int itoa(int value, char *sp, int radix)
{
    char tmp[16];// be careful with the length of the buffer
    char *tp = tmp;
    int i;
    unsigned v;

    int sign = (radix == 10 && value < 0);    
    if (sign)
        v = -value;
    else
        v = (unsigned)value;

    while (v || tp == tmp)
    {
        i = v % radix;
        v /= radix; // v/=radix uses less CPU clocks than v=v/radix does
        if (i < 10)
          *tp++ = i+'0';
        else
          *tp++ = i + 'a' - 10;
    }

    int len = tp - tmp;

    if (sign) 
    {
        *sp++ = '-';
        len++;
    }

    while (tp > tmp)
        *sp++ = *--tp;

    return len;
}

///////////////////////////////////////////////////////////////////////////////

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

// store the given MAC address to a FILE into the SD card
void storeConfigToSD() {
  FILE *fp = fopen(config_path, "w");
  if (fp == NULL) {
    printf("Error opening file!\n");
    exit(1);
  }
  Log::i("Store lat lon mac: %s %s %s", configGal.lat, configGal.lon, mac_string);
  fprintf(fp,"deviceid:%s\nlat:%s\nlon:%s",mac_string,configGal.lat,configGal.lon);
  fclose(fp);
}


void showThresholdValues(){
    Log::i("Calibration on SD ended");
    Log::i("Positive thresholds X:%lf Y:%lf Z:%lf", pthresx, pthresy, pthresz);
    Log::i("Negative thresholds X:%lf Y:%lf Z:%lf", nthresx, nthresy, nthresz);

}

void ipToString(char* buf, int maxsize, IPAddress addr) {
  snprintf(buf, maxsize, "%i.%i.%i.%i", addr[0], addr[1], addr[2], addr[3]);
}

void printConfig(){
  Log::i("###################### Config ######################### ");
  Log::i("UDID (DeviceID): %s - Model: %s - Version: %s", mac_string, configGal.model, configGal.version);
  Log::i("Position (lat, lon): %lf %lf", configGal.lat, configGal.lon);

  char buf[300];
  IPAddress localIp = Ethernet.localIP();
  snprintf(buf, 300, "%i.%i.%i.%i", localIp[0], localIp[1], localIp[2], localIp[3]);

  Log::i("IP: %s - Connection errors: %i", buf, errors_connection);
  showThresholdValues();
  Log::i("##################### Config end ####################### ");
}

void resetBlink(byte type){
	if(type){ // if 1 reset
		digitalWrite(red_Led, HIGH);
		digitalWrite(green_Led, LOW);
		delay(500);
		digitalWrite(red_Led, LOW);
		digitalWrite(green_Led, HIGH);
		delay(500);
		digitalWrite(red_Led, HIGH);
		digitalWrite(green_Led, LOW);
		delay(500);
		digitalWrite(red_Led, LOW);
		digitalWrite(green_Led, HIGH);
		delay(1500);
	}else{ // if 0 update
		digitalWrite(red_Led, HIGH);
		digitalWrite(green_Led, HIGH);
		delay(500);
		digitalWrite(red_Led, LOW);
		digitalWrite(green_Led, LOW);
		delay(500);
		digitalWrite(red_Led, HIGH);
		digitalWrite(green_Led, HIGH);
		delay(500);
		digitalWrite(red_Led, LOW);
		digitalWrite(green_Led, LOW);
		delay(1000);
	}
}

#endif 
