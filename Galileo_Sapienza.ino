#if ARDUINO < 153
#else
#define __IS_GALILEO
#endif

#include <Ethernet.h>
#include <SPI.h>
#include <EEPROM.h>
#include <SD.h>
#include <math.h>

#ifdef __IS_GALILEO
//#include <Ethernet.h>
#include <EthernetUdp.h>
#include <sys/sysinfo.h>
#include <signal.h>
#include <stdlib.h>  // for ntp_alt.h
#endif

double pthresx = 0;
double pthresy = 0;
double pthresz = 0;
double nthresx = 0;
double nthresy = 0;
double nthresz = 0;

unsigned long freeRam();

long previousMillis = 0;        // will store last time LED was updated
long interval = 2*30*1000;
long previousMillisNTP = 0;        // will store last time LED was updated
long intervalNTP = 15*60*1000;

#include "AcceleroMMA7361.h"
#include "config.h"
//#include "ntp.h"
#include "ntp_alt.h"
#include "localstream.h"
#include "threshold.h"
#include "avg.h"
#include "httpconn.h"
#include "cfgupdate.h"
#include "GalileoLog.h"

//definition freeRam method
#ifdef __IS_GALILEO
unsigned long freeRam() {
  struct sysinfo s;
  sysinfo(&s);
  return s.freeram;
}

static struct RECORD ddl = {1000, 3600, 1020, 400, 200};
struct RECORD *db = &ddl;
unsigned long currentMillis, milldelayTime;


#else
#if (ARDUINO >= 100)
#include <Arduino.h>
#else
#include <WProgram.h>
#endif
//int freeRam () {
//  extern int __heap_start, *__brkval; 
//  int v; 
//  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
//}
#endif

//definition of Accelerometer object
AcceleroMMA7361 accelero;
int currentHour = -1;

// return true if at least one of the axis is over the threshold
boolean isOverThreshold(struct RECORD *db, struct TDEF *td) {
  return (db->valx > td->pthresx || db->valx < td->nthresx)
      || (db->valy > td->pthresy || db->valy < td->nthresy)
      || (db->valz > td->pthresz || db->valz < td->nthresz);
}

void checkSensore() 
{
  int valx, valy, valz;
  
  valx = accelero.getXAccel();
  valy = accelero.getYAccel();
  valz = accelero.getZAccel();
      
  TDEF td = { pthresx, pthresy, pthresz, nthresx, nthresy, nthresz };
  
  //struct RECORD *db = (struct RECORD*)malloc(sizeof(struct RECORD));
  db->ts = getUNIXTime();
  db->ms = getUNIXTimeMS();
  db->valx = getAvgX(valx);
  db->valy = getAvgY(valy);
  db->valz = getAvgZ(valz);
  db->overThreshold = isOverThreshold(db, &td);
   
  if (isConnected) {
  	sendValues(db);  // send the values of the accelerometer to the mobile APP (if the APP is listening)
  }
  
  // if the values of the accelerometer have passed the threshold
  //  or if an "event" is currently running
  if (db->overThreshold || inEvent == 1) {
    if (isConnected) {
      if (ledON) digitalWrite(10,HIGH);
      httpSendValues(db, &td);
      //Serial.println("IN EVENT - __CONNECTED__");
    }else {
      //saveToSDhttpSendValues();  // not yet implemented
      //free(db); // Memory leak debugged
      if (ledON) digitalWrite(10,LOW);
      //Serial.println("freeing memory for db");
      //Serial.println("IN EVENT - BUT NOT CONNECTED");
    }
  }
  else {
      //free(db); // Memory leak debugged
      if (ledON) digitalWrite(10,LOW);
      //Serial.println("freeing memory for db");
      //Serial.println("NOT IN EVENT");
   }
}

void debug_Axis() {  // Reading sensor to view what is measuring. For Debug Only
  int valx, valy, valz;
  
  valx = accelero.getXAccel();
  valy = accelero.getYAccel();
  valz = accelero.getZAccel();
  if (debugON) {
    Serial.print("Valori Accelerometro:  ");
    Serial.print(valx);
    Serial.print("   ");
    Serial.print(valy);
    Serial.print("   ");
    Serial.println(valz);
  }
}

// set up the ethernet connection per location;
// use the linux underneath the device to force manual DNS and so on
void setupEthernet() {
	switch (deviceLocation) {
			case 0:  // Sapienza Colossus
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

			case 1:  // Panizzi's room
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

			case 2:  // Home
				timeServer = IPAddress(132, 163, 4, 101);
				if (isDhcpEnabled) {
					boolean isDhcpWorking = false;
					while (!isDhcpWorking) { // WARNING: add DHCP timeout
						/* Trying to get an IP address */
						if (Ethernet.begin(mac) == 0) {
							if (debugON) Serial.println("Error while attempting to get an IP, retrying in 5 seconds...");
							delay(5000);
						}
						else {
							if (debugON) Serial.print("IP retrived successfully from DHCP: ");
							if (debugON) Serial.println(Ethernet.localIP());
							isDhcpWorking = true;
						}
					}
				}
				else {
						// add your static IP here
						//ip = IPAddress(192, 168, 1, 36);
						Ethernet.begin(mac); // controllare errore
						//system("ifconfig eth0 192.168.1.36");  // fixed ip address to use the telnet connection
						system("ifconfig > /dev/ttyGS0");  // debug
				}
				break;

			default:  // like case 0, Sapienza Colossus
				Ethernet.begin(mac, ip, dns, gateway);
				system("ifconfig eth0 10.10.1.101 netmask 255.255.255.0 up > /dev/ttyGS0 < /dev/ttyGS0");  // set IP and SubnetMask for the Ethernet
				system("route add default gw 10.10.1.1 eth0 > /dev/ttyGS0 < /dev/ttyGS0");  // change the Gatway for the Ethernet
				system("echo 'nameserver 151.100.17.18' > /etc/resolv.conf");  // add the DNS
	}
}

void setup() {
#ifdef __IS_GALILEO
  // Fixing Arduino Galileo bug
  signal(SIGPIPE, SIG_IGN);
        // Workaround for Galileo (and other boards with Linux)
      system("/etc/init.d/networking restart");
      delay(1000);
      // Remove for production use
      system("telnetd -l /bin/sh");
  #endif
  //system("/etc/init.d/networking restart");
  //delay(1000);
  //system("telnetd -l /bin/sh");
  Serial.begin(9600);
  delay(500);

  Serial.println("#############INITIALIZING DEVICE#############\n");
  if (logON) log("#############INITIALIZING DEVICE#############\n");
  /* Calibrating Accelerometer */
  accelero.begin(13, 12, 11, 10, A0, A1, A2);     // set the proper pin x y z
  accelero.setSensitivity(LOW);                  // sets the sensitivity to +/-6G
  if (debugON) Serial.println("calibrate()");
  accelero.calibrate();
  accelero.setAveraging(1);  // number of samples that have to be averaged
  if (debugON) Serial.println("setAveraging(1)");
//  #ifdef __IS_GALILEO
//	  // Workaround for Galileo (and other boards with Linux)
//	  system("/etc/init.d/networking restart");
//          delay(1000);
//  #endif

  if (debugON) Serial.println("Setting up ethernet connection");
  // Config connction on Ethernet module
  setupEthernet();
  isConnected = isConnectedToInternet();
  
  Serial.print("STATUS CONNECTION: ");
  Serial.println(isConnected ? "CONNECTED" : "NOT CONNECTED");
  delay(500);  
  
  if (ledON) {
  	pinMode(12, OUTPUT);
		if (isConnected) digitalWrite(12,HIGH);
		digitalWrite(10,LOW);
		pinMode(10, OUTPUT);
		digitalWrite(10,LOW);
  }

  //system("cat /etc/resolv.conf > /dev/ttyGS0 < /dev/ttyGS0");  // DEBUG
  
  if (debugON) Serial.println("Forcing config update...");

  initConfigUpdates();
  
  if (debugON) Serial.println("Syncing NTP...");
  initNTP();
  // We need to set this AFTER ntp sync...
  lastCfgUpdate = getUNIXTime();
  
  if (debugON) Serial.println("EEPROM init");
  initEEPROM();
  
  if (debugON) Serial.println("UDP Command Socket init");
  commandInterfaceInit(); // open port for mobile app
  
  if (debugON) Serial.print("Free RAM: ");
  if (debugON) Serial.println(freeRam()); //debug
  
  if (debugON) Serial.println("\n#############INIZIALIZATION COMPLETE!#############");
  milldelayTime = millis();
}

void loop() {
	currentMillis = millis();
	// debug only, check if the sketch is still running
	if (currentMillis - previousMillis > interval) {
		previousMillis = currentMillis;
		isConnected = isConnectedToInternet();
		if (!isConnected) {
			if (ledON) digitalWrite(12,LOW);
		}
		else {
			if (ledON) digitalWrite(12,HIGH);
		}
                 
		log("Still running");
		if (debugON) {
			Serial.print("Still running__INTERVAL: ");
			Serial.println(interval);
			Serial.print("STATUS CONNECTION: ");
			Serial.println(isConnected?"CONNECTED":"NOT CONNECTED");
		}
	}

	// sync with the NTP server
	unsigned long currentMillisNTP = millis();
	if (currentMillisNTP - previousMillisNTP > intervalNTP) {
		previousMillisNTP = currentMillisNTP;
		NTPdataPacket();
	}

  //doNTPActions();
  //delay(50);
  if (millis() - milldelayTime > 60) {
		doConfigUpdates();

		int cHour = (getUNIXTime() % 86400L) / 3600;
		if (currentHour != cHour) {
			currentHour = cHour;
			if (debugON) Serial.println("checkCalibrationNeeded");
			checkCalibrationNeeded(accelero, cHour);
		}

		checkCommandPacket();

		checkSensore();
		//testNTP();
		milldelayTime = millis();
  }
}
