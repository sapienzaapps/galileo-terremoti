#if ARDUINO < 153
#else
	#define __IS_GALILEO
#endif

#include <Ethernet.h>
#include <SPI.h>
#include <EEPROM.h>
#include <SD.h>
#include <math.h>
#define GEN1 1 // 1 if gen1 else 0
#define GEN2 0 // remove if gen1 = 1

#ifdef __IS_GALILEO
	#include <EthernetUdp.h>
	#include <sys/sysinfo.h>
	#include <signal.h>
	#include <stdlib.h>  // for ntp_alt.h
#endif
//Actual versions: 1.7 gen1   1.7 gen2


byte zz = 0;
// accelerometer values
double pthresx = 0;
double pthresy = 0;
double pthresz = 0;
double nthresx = 0;
double nthresy = 0;
double nthresz = 0;

bool resetEthernet = false;
unsigned long freeRam();

unsigned long previousMillis = 0;        // will store last time LED was updated
unsigned long previousMillisNTP = 0;     // will store last time LED was updated
unsigned long pingIntervalCheckCounter = 0;
unsigned long calibrationInterval = 60*60*1000;
unsigned long prevMillisCalibration = 0;
unsigned long TimeEvent = 60*1000;// 60 second per Event
unsigned long milldelayTimeEvent = 0;
unsigned long startRecord = 0;
unsigned long millis24h;
unsigned long lastRstMill = 0;
int resetnum = 0;
int numAlert = 0;
int reTareNum = 40;

// const byte red_Led = 10;
// const byte green_Led = 12;
const byte yellow_Led = 8;
const byte red_Led = 12;
const byte green_Led = 10;
bool redLedStatus = false;
bool greenLedStatus = false;
bool yellowLedStatus = false;
bool chekInternet = false;
bool recordData = false;
byte errors_connection = 0;


#include "AcceleroMMA7361.h"
#include "GalileoLog.h"
#include "commons.h"
#include "ntp_alt.h"
#include "localstream.h"
#include "threshold.h"
#include "avg.h"
#include "httpconn.h"
#include "cfgupdate.h"

//definition freeRam method
#ifdef __IS_GALILEO
	unsigned long freeRam() {
		struct sysinfo s;
		sysinfo(&s);
		return s.freeram;
	}

	// static temp struct
	static struct RECORD ddl = {0, 0, 0, 0, 0, false};
	struct RECORD *db = &ddl;
	unsigned long currentMillis, milldelayTime;
#else
#if (ARDUINO >= 100)
	#include <Arduino.h>
#else
	#include <WProgram.h>
#endif
#endif

//definition of Accelerometer object
AcceleroMMA7361 accelero;
int currentHour = -1;

// return true if at least one of the axis is over the threshold
boolean isOverThresholdBasic(struct RECORD *db, struct TDEF *td) {
  return (db->valx > td->pthresx || db->valx < td->nthresx)
      || (db->valy > td->pthresy || db->valy < td->nthresy)
      || (db->valz > td->pthresz || db->valz < td->nthresz);
}

boolean isOverThresholdFixed(struct RECORD *db, struct TDEF *td) {
  return (abs(db->valx) > td->pthresx)
      || (abs(db->valy) > td->pthresy)
      || (abs(db->valz - gForce) > td->pthresz);
}

void checkSensore() 
{
  int valx, valy, valz;
  
  valx = accelero.getXAccel();
  valy = accelero.getYAccel();
  valz = accelero.getZAccel();
  
  TDEF td = { pthresx, pthresy, pthresz, nthresx, nthresy, nthresz };
  
  //struct RECORD *db = (struct RECORD*)malloc(sizeof(struct RECORD));
  //db->ts = getUNIXTime();
  // db->ms = getUNIXTimeMS(); !!!!!!! necessary for mobile app!!!!!!!!!! sendValues()
  db->valx = getAvgX(valx);
  db->valy = getAvgY(valy);
  db->valz = getAvgZ(valz);
  //db->overThreshold = isOverThresholdBasic(db, &td);
  db->overThreshold = false;


  switch(thresholdAlghoritm) {
  	case Basic:
  		db->overThreshold = isOverThresholdBasic(db, &td);
  		break;

  	case Fixed:
  		db->overThreshold = isOverThresholdFixed(db, &td);
  		break;

  	default:  // Basic
  		db->overThreshold = isOverThresholdBasic(db, &td);
  }

  //debug_Axis(); // Debug Only
   
  //if (internetConnected || !testNoInternet ) {
  	sendValues(db);  // send the values of the accelerometer to the mobile APP (if the APP is listening)
    //delay(1);
  //}
  
  // if the values of the accelerometer have passed the threshold
  //  or if an "event" is currently running
  //if (db->overThreshold || inEvent != 1) {
  if (db->overThreshold && inEvent != 1) {
    db->ms = getUNIXTimeMS();
  
    Serial.println(db->overThreshold?"overThreshold":"inEvent");
    if (ledON && !redLedStatus){
        digitalWrite(red_Led,HIGH);
        redLedStatus = !redLedStatus;
    }
    if (internetConnected && testNoInternet && start) {// send value data if there is a connection
      //if (ledON) digitalWrite(green_Led,HIGH);
      if(alert){
        inEvent = 1;
        milldelayTimeEvent = millis(); // timestamp in millis for Event Interval */
        
        httpSendAlert1(db, &td);
        numAlert++;
        //getConfigNew();// on testing
        
      }else{
        Serial.println("IN EVENT - BUT NOT ADJUSTED SENDVALUES");
        //httpSendValues(db, &td);
        //Serial.println("IN EVENT - __CONNECTED__");
        }
    }
    else {
      //saveToSDhttpSendValues();  // not yet implemented
      //free(db); // Memory leak debugged
      //if (ledON) digitalWrite(green_Led,LOW);
      //if (logON) log("NOT CONNECTED");
      if (debugON) Serial.println("NOT CONNECTED - or not in start!");
      //Serial.println("freeing memory for db");
      //Serial.println("IN EVENT - BUT NOT CONNECTED");
      
      // STORE FILE ON SD CARD
      // sd = active
    }
  }
  else {
      //free(db); // Memory leak debugged
      if ( ledON && !(inEvent) && redLedStatus ){
        digitalWrite(red_Led,LOW);
        redLedStatus = !redLedStatus;
        }

      //if (ledON) digitalWrite(green_Led,LOW);
      //Serial.println("freeing memory for db");
      //Serial.println("NOT IN EVENT");
   }
 
  db->ts = 0;
  db->ms = 0;
  db->valx = 0;
  db->valy = 0;
  db->valz = 0;
  db->overThreshold = false;

   
}

void debug_Axis() {  // Reading sensor to view what is measuring. For Debug Only
  if(recordData){// if recording data -> get accelero data
    int _valx, _valy, _valz;
    accelero.getAccelXYZ(&_valx, &_valy, &_valz) ;
    if(zz == 0 ){ // first time record
      resetBlink(0);
      logAccValues( 0,  0,  0, zz);
      zz = 1;
    }else if(zz == 1){ // RECORDING 
      logAccValues( _valx,  _valy,  _valz, zz );
    }else if(zz == 2 ){ // last time record
      logAccValues( 0,  0,  0, zz);
      zz = 3;
      resetBlink(0);
    }
  }else{
    // _valx = accelero.getXAccel();
    // _valy = accelero.getYAccel();
    // _valz = accelero.getZAccel();
    // delay(1);

    if (debugON /*&& db->overThreshold*/) {
      Serial.print("Valori Accelerometro:  ");
      Serial.print(db->valx);
      Serial.print("   ");
      Serial.print(db->valy);
      Serial.print("   ");
      Serial.println(db->valz);
    }  
    // if (debugON /*&& db->overThreshold*/) {
      // Serial.print("Valori Accelerometro:  ");
      // Serial.print(_valx);
      // Serial.print("   ");
      // Serial.print(_valy);
      // Serial.print("   ");
      // Serial.println(_valz);
    // }
  }
}

// set up the ethernet connection per location;
// use the linux underneath the device to force manual DNS and so on
void setupEthernet() {
  switch (deviceLocation) {
	  case Colossus:  // Sapienza Colossus
	    ip = IPAddress(10, 10, 1, 101);
	    dns = IPAddress(151, 100, 17, 18);
	    gateway = IPAddress(10, 10, 1, 1);
	    subnet = IPAddress(255, 255, 255, 0);
      Ethernet.begin(mac, ip, dns, gateway); //vedere
	    timeServer = IPAddress(10, 10, 1, 1);
      system("ifconfig eth0 10.10.1.101 netmask 255.255.255.0 up > /dev/ttyGS0 < /dev/ttyGS0");  // set IP and SubnetMask for the Ethernet
	    system("route add default gw 10.10.1.1 eth0 > /dev/ttyGS0 < /dev/ttyGS0");  // change the Gatway for the Ethernet
	    system("echo 'nameserver 151.100.17.18' > /etc/resolv.conf");  // add the DNS
	    break;

	  case Panizzi:  // Panizzi's room
	    ip = IPAddress(151, 100, 17, 143);
	    dns = IPAddress(151, 100, 17, 18);
	    gateway = IPAddress(151, 100, 17, 1);
	    subnet = IPAddress(255, 255 ,255 ,0);
      Ethernet.begin(mac, ip, dns, gateway);
	    timeServer = IPAddress(37, 247, 49, 133);
      system("ifconfig eth0 151.100.17.143 netmask 255.255.255.0 up > /dev/ttyGS0 < /dev/ttyGS0");  // set IP and SubnetMask for the Ethernet
	    system("route add default gw 151.100.17.1 eth0 > /dev/ttyGS0 < /dev/ttyGS0");  // change the Gatway for the Ethernet
	    system("echo 'nameserver 151.100.17.18' > /etc/resolv.conf");  // add the DNS
      break;

	  case Home:  // Home
	    /* timeServer = IPAddress(132, 163, 4, 101); */
	    timeServer = IPAddress(178, 33, 50, 131);
	    if (isDhcpEnabled) {
	      boolean isDhcpWorking = false;
	      while (!isDhcpWorking) { // WARNING: add DHCP timeout
          /* Trying to get an IP address */
          if (Ethernet.begin(mac) == 0) {  // Error retrieving DHCP IP 
            if (debugON) Serial.println("Error while attempting to get an IP, retrying in 5 seconds...");
            delay(5000);
          }else {  // DHCP IP retireved successfull
            if (debugON) Serial.print("IP retrived successfully from DHCP: ");
            if (debugON) Serial.println(Ethernet.localIP());
            isDhcpWorking = true;
          }
        }
         break;
	    }
	    else { // Home Static Configuration 
        if (debugON) Serial.println("Static Configuration");
        if (logON) log("Static Configuration\n");
	      //add your static IP here
        if (GEN1) ip = IPAddress(192, 168, 1, 177);// gen1
        else ip = IPAddress(192, 168, 1, 178); // gen2
  	    //dns = IPAddress(192,168,1,1);
  	    dns = IPAddress(192,168,1,254);
  	    //gateway = IPAddress(192,168,1,1);
  	    gateway = IPAddress(192,168,1,254);
        subnet = IPAddress(255, 255 ,255 ,0);
        // ARDUINO START CONNECTION		
	      Ethernet.begin(mac, ip, dns, gateway, subnet); // Static address configuration 
        //LINUX SYSTEM START CONNECTION
        if (GEN1) system("ifconfig eth0 192.168.1.177 netmask 255.255.255.0 up");  // set IP and SubnetMask for the Ethernet
        else system("ifconfig eth0 192.168.1.178 netmask 255.255.255.0 up");  // set IP and SubnetMask for the Ethernet
        //else system("ifconfig eth0 192.168.1.178 netmask 255.255.255.0 up > /dev/ttyGS0 < /dev/ttyGS0");  // set IP and SubnetMask for the Ethernet
	      //system("route add default gw 192.168.1.254 eth0 > /dev/ttyGS0 < /dev/ttyGS0");  // change the Gateway for the Ethernet
	      system("route add default gw 192.168.1.254 eth0");  // change the Gateway for the Ethernet
	      system("echo 'nameserver 8.8.8.8' > /etc/resolv.conf");  // add the GOOGLE DNS
				//system("ifconfig eth0 192.168.1.36");  // fixed ip address to use the telnet connection
	      //system("ifconfig > /dev/ttyGS0");  // debug
        if (debugON) {
          Serial.print("Static IP: ");
          Serial.println(Ethernet.localIP());
        }
        break;
      }// END - Home Static Configuration 
	    break;

	  default:  // like case 0, Sapienza Colossus
	    Ethernet.begin(mac, ip, dns, gateway);
	    system("ifconfig eth0 10.10.1.101 netmask 255.255.255.0 up > /dev/ttyGS0 < /dev/ttyGS0");  // set IP and SubnetMask for the Ethernet
	    system("route add default gw 10.10.1.1 eth0 > /dev/ttyGS0 < /dev/ttyGS0");  // change the Gatway for the Ethernet
	    system("echo 'nameserver 151.100.17.18' > /etc/resolv.conf");  // add the DNS
      system("ifconfig > /dev/ttyGS0");  // debug
  }// END switch device location
}// END SetupEthernet()

void setup() {
  analogReadResolution(10);        // 3.3V => 4096
  Serial.print("    Res12bit: ");
  delay(1000);
  Serial.begin(9600);
  delay(500);
  Serial.println("Starting.........");
	#ifdef __IS_GALILEO
    // Fixing Arduino Galileo bug
    //signal(SIGPIPE, SIG_IGN); Removed - caused not restarting sketch
    // Workaround for Galileo (and other boards with Linux)
    system("/etc/init.d/networking restart");
    delay(1000);
    // Remove for production use
    system("telnetd -l /bin/sh");
  #endif
  //system("/etc/init.d/networking restart");
  //delay(1000);
  //system("telnetd -l /bin/sh");
  /* Serial.begin(9600); */

  //Serial.println("STOREConfig()");
  //storeConfigToSD();
  //delay(300);
  Serial.println("#############INITIALIZING DEVICE#############\n");
  if (logON) log("###INITIALIZING DEVICE###");
  //if (logON) log("###INITIALIZING DEVICE###");
  Serial.println("readConfig()");
  readConfig(); // read config from SD Card
  /* Calibrating Accelerometer */
  accelero.begin(/* 13, 12, 11, 10, */ A0, A1, A2);     // set the proper pin x y z
  //accelero.setSensitivity(LOW);                  // sets the sensitivity to +/-6G
  if (debugON) Serial.println("calibrate()");
  accelero.setAveraging(10);  // number of samples that have to be averaged
  accelero.calibrate();
  if (debugON) Serial.println("setAveraging(10)");
  // Config connection on Ethernet module
  if (debugON) Serial.println("Setting up ethernet connection");
  setupEthernet();
  delay(1500);
  //byteMacToString(mac); // create string for MAC address
  if (request_mac_from_server) {
    Serial.println("Requesting deviceID to server... ");
		getMacAddressFromServer(); // asking for new mac address/deviceid
    HEXtoDecimal(mac_string, strlen(mac_string), mac);
    byteMacToString(mac);
	// convertMACFromStringToByte();
	}
  if(!request_lat_lon) start = true;

  internetConnected = isConnectedToInternet();
  delay(500); // wait internet connaction Status 
  Serial.print("STATUS CONNECTION: ");
  Serial.println(internetConnected ? "CONNECTED" : "NOT CONNECTED");
  delay(500);  
  
  if (ledON) {
  	pinMode(green_Led, OUTPUT);
		digitalWrite(green_Led,HIGH);
    delay(500);
		digitalWrite(green_Led,LOW);    
		if (internetConnected){
      digitalWrite(green_Led,HIGH);
      greenLedStatus = true;
    }else greenLedStatus = false;
    
		digitalWrite(red_Led,HIGH);
    delay(500);
    digitalWrite(red_Led,LOW);
		pinMode(red_Led, OUTPUT);
		digitalWrite(red_Led,LOW);
    redLedStatus = false;
   	pinMode(yellow_Led, OUTPUT);
		digitalWrite(yellow_Led,HIGH);  
    delay(500);
		digitalWrite(yellow_Led,LOW);  
  }

  //system("cat /etc/resolv.conf > /dev/ttyGS0 < /dev/ttyGS0");  // DEBUG
  if (debugON) Serial.println("Forcing config update...");
  initConfigUpdates();
  if (debugON) Serial.println("EEPROM init");
  //initThrSD(false);
  //initEEPROM(forceInitEEPROM);
  if (debugON) Serial.println("UDP Command Socket init");
  commandInterfaceInit(); // open port for mobile app
  
  if(internetConnected && testNoInternet){
    if (debugON) Serial.println("Syncing NTP...");
    initNTP(); // controllare il ritorno ++++++++++++++++++++++++++++++++++++++++++++
    // We need to set this AFTER ntp sync...
    lastCfgUpdate = getUNIXTime();
  }
  
  if(doesFileExist(script_reset)!= 1){ // check if reset script exists
    if (debugON) Serial.println("createScript...");
    createScript(script_reset, reboot_scriptText); 
    //if (debugON) Serial.println("execScript...");
    //execScript(script_path);
  }
  
  if (debugON) Serial.print("Free RAM: ");
  if (debugON) Serial.println(freeRam()); //debug
  
  if (debugON) Serial.println("\n#############INIZIALIZATION COMPLETE!#############");
  //if (debugON) Serial.println("\n#############strMacToByte#############");
  //byte mac2[] ={ 0x00, 0x13, 0x20, 0xFF, 0x15, 0x9F };
/*   HEXtoDecimal(mac_string, strlen(mac_string), mac2);
  Serial.println("");
  Serial.println("Testttttttt");
   for(int z=0; z<6; z++){
     if (mac2[z] < 0x10) Serial.print(0);
    
    Serial.print(mac2[z],HEX);
    Serial.print(":");
  } 
  Serial.println(""); */
  // char* latfinta = "45.000321";
  // stringToDouble(latfinta);
  Serial.println("Testttttttt - FINITOOOOO");
  
  
  millis24h =  millis();
  milldelayTime = millis24h;
  testNoInternet = true; // true = NOT IN TEST -- false = IN TEST
}
// end SETUP

void loop() {
	currentMillis = millis();
	// debug only, check if the sketch is still running and Internet is CONNECTED
	if ((testNoInternet) && (currentMillis - previousMillis > checkInternetConnectionInterval) || chekInternet/*|| resetConnection*/) {
		if(chekInternet) chekInternet = false;
    internetConnected = isConnectedToInternet();
		if (!internetConnected) {
      //internetConnected = isConnectedToInternet();
			if (ledON && greenLedStatus){
        digitalWrite(green_Led,LOW);
        greenLedStatus = !greenLedStatus;
      }
      resetEthernet = true;
		}
		else { // IF INTERNET IS PRESENT 
      if (ledON && !greenLedStatus){
        digitalWrite(green_Led,HIGH);
        greenLedStatus = !greenLedStatus;
      }
      //doConfigUpdates(); // controllare +++++++++++++++++++++++++++++++++++++++++
      resetEthernet = false;
      // if (start)getConfigNew(); // CHEK FOR UPDATES
     
		}
		if(logON)log("Still running");
		if (debugON) {
			Serial.print("Loop - Still running: ");
      Serial.println(getGalileoDate());
			Serial.print("CHECK INTERNET INTERVAL: ");
			Serial.println(checkInternetConnectionInterval);
			Serial.print("STATUS CONNECTION: ");
			Serial.println(internetConnected?"CONNECTED":"NOT CONNECTED");
		}
		previousMillis = currentMillis;
	}

  // sync with the NTP server
	// unsigned long currentMillisNTP = millis();
	if ((testNoInternet) &&(currentMillis - previousMillisNTP > NTPInterval) && internetConnected && !resetEthernet) {
	  NTPdataPacket();
    execSystemTimeUpdate();
	  previousMillisNTP = currentMillis;
	}
  
  // Check for calibration Sensor
  // unsigned long currentMillisCalibration = millis();
	if (testNoInternet && (currentMillis - prevMillisCalibration > calibrationInterval) || ForceCalibrationNeeded) {
    int cHour = (getUNIXTime() % 86400L) / 3600;
	  if (debugON) Serial.println("checkCalibrationNeeded---------------------------------------");
	  // checkCalibrationNeeded(accelero, cHour);
    checkCalibrationNeededNOSD(accelero, cHour);
    ForceCalibrationNeeded = false;
    prevMillisCalibration = currentMillis;
  }
if(numAlert >= reTareNum){
  Serial.println(" alert >40 recalibrating......");  
  accelero.calibrate();
  forceInitEEPROM = true;
  int cHour = (getUNIXTime() % 86400L) / 3600;
  checkCalibrationNeededNOSD(accelero, cHour);
  numAlert = 0;
}  
  if (inEvent) {// unlock inEvent time
    // unsigned long millisTimeEvent = millis();
    if (currentMillis - milldelayTimeEvent > nextContact /* TimeEvent */) { // unlock Alert after xxx millisecs
      inEvent = 0;
    }
  }
  if (resetEthernet && testNoInternet/* || errors_connection > 10 */) {
    // unsigned long ethRstMill = millis();
    if (currentMillis - lastRstMill > 120000UL) {
      if(logON) log("networking restart - NOT CONNECTED FINTO!!!");
      if(debugON) Serial.println("networking restart!!!");
      // Workaround for Galileo (and other boards with Linux)
      system("/etc/init.d/networking restart");
      
      delay(2000);
      setupEthernet();
      delay(1000);
      lastRstMill = currentMillis;
      chekInternet = true;
      resetnum++;
      if (resetnum > 2){ 
        if(debugON) Serial.println("SYSTEM restart!!!");
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
        //execScript(script_reset);
        system("reboot");
        //system("shutdown -r -t sec 00");
        for(;;)
        ;
      }
    }
  } 

  if (currentMillis - milldelayTime > 50UL /* checkSensoreInterval */ && zz < 3 ) { // read sensor values
    if(!recordData){
    // check mobile command
      checkCommandPacket();
    // read sensor values
      checkSensore();
    }else{
      // record acc data for 1h
      if (zz == 0){
        startRecord = currentMillis;
        Serial.println("#########################");
        Serial.println("###############STARTING RECORD----------------------------------------");
      }
      debug_Axis();

      if (currentMillis - startRecord >= 3600000UL && zz == 1/* 3600000UL */){ // AFTER TIME RECORD
        zz = 2;
        Serial.println("#########################");
        Serial.println("###############STOPPING RECORD----------------------------------------");
      }
    }
		milldelayTime = currentMillis;
  }
  
  // check if is time to update config
  if (/* start && */ testNoInternet && internetConnected){ 
    if (currentMillis - lastCfgUpdate > checkConfigInterval){
      getConfigNew(); // CHEK FOR UPDATES
      lastCfgUpdate = currentMillis;
      if (debugON) printConfig();
    }
  
  }
  
  // reset the device every 24h or after 20 errors
  if (((currentMillis - millis24h >=  86400000UL * 2  ) && !inEvent /* && !inEventSD */) || (errors_connection > 20)){
    log("Reboot after 24h of activity");
    if (debugON) Serial.println("<---------- Reboot after 24h of activity ----------->");
      resetBlink(1);
      execScript(script_reset);
    //system("reboot");
    //delay(500);
    for(;;){
      resetBlink(1);
    }
  }
  //delay(1);
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
