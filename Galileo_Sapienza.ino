#if ARDUINO < 153
#else
#define __IS_GALILEO
#endif

#include <math.h>
#include <SPI.h>
#include <Ethernet.h>
#include <EEPROM.h>
#include <SD.h>

#ifdef __IS_GALILEO
//#include <Ethernet.h>
#include <EthernetUdp.h>
#include <sys/sysinfo.h>
#include <signal.h>
#endif

double pthresx = 0;
double pthresy = 0;
double pthresz = 0;
double nthresx = 0;
double nthresy = 0;
double nthresz = 0;

unsigned long freeRam();

#include "AcceleroMMA7361.h"
#include "config.h"
#include "ntp.h"
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
#else
#include <MemoryFree.h> //add .h and cpp MemoryFree for Arduino based only
unsigned long freeRam() {
	return (unsigned long)freeMemory();
	}
#endif

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
  
  struct RECORD *db = (struct RECORD*)malloc(sizeof(struct RECORD));
  db->ts = getUNIXTime();
  db->ms = getUNIXTimeMS();
  db->valx = getAvgX(valx);
  db->valy = getAvgY(valy);
  db->valz = getAvgZ(valz);
  db->overThreshold = isOverThreshold(db, &td);
  
  sendValues(db);  // send the values of the accelerometer to the mobile APP (if the APP is listening)
  
  if(db->overThreshold || inEvent == 1)  // if the values of the accelerometer have passed the threshold or if an "event" is currently running
  {
    httpSendValues(db, &td);
  }
  
}

// set up the ethernet connection per location;
// use the linux underneath the device to force manual DNS and so on
void setupEthernet() {
	switch (deviceLocation) {
			case 0:  // Sapienza Colossus
				ip = IPAddress(10,10,1,101);
				dns = IPAddress(151,100,17,18);
				gateway = IPAddress(10,10,1,1);
				subnet = IPAddress(255,255,255,0);

				Ethernet.begin(mac, ip, dns, gateway);
				timeServer = IPAddress(10, 10, 1, 1);

				system("ifconfig eth0 10.10.1.101 netmask 255.255.255.0 up > /dev/ttyGS0 < /dev/ttyGS0");  // set IP and SubnetMask for the Ethernet
				system("route add default gw 10.10.1.1 eth0 > /dev/ttyGS0 < /dev/ttyGS0");  // change the Gatway for the Ethernet
				system("echo 'nameserver 151.100.17.18' > /etc/resolv.conf");  // add the DNS
				break;

			case 1:  // Panizzi's room
				ip = IPAddress(151,100,17,143);
				dns = IPAddress(151,100,17,18);
				gateway = IPAddress(151,100,17,143);
				subnet = IPAddress(255,255,255,0);

				Ethernet.begin(mac, ip, dns, gateway);
				timeServer = IPAddress(37, 247, 49, 133);

				system("ifconfig eth0 151.100.17.143 netmask 255.255.255.0 up > /dev/ttyGS0 < /dev/ttyGS0");  // set IP and SubnetMask for the Ethernet
				system("route add default gw 151.100.17.1 eth0 > /dev/ttyGS0 < /dev/ttyGS0");  // change the Gatway for the Ethernet
				system("echo 'nameserver 151.100.17.18' > /etc/resolv.conf");  // add the DNS
				break;

			case 2:  // Home
				timeServer = IPAddress(37, 247, 49, 133);
				if (isDhcpEnabled) {
					boolean isDhcpWorking = false;
					while(!isDhcpWorking) {
						/* Trying to get an IP address */
						if (Ethernet.begin(mac) == 0) {
							Serial.println("Error while attempting to get an IP, retrying in 5 seconds...");
							delay(5000);
						} else {
							Serial.print("IP retrived successfully from DHCP: ");
							Serial.println(Ethernet.localIP());
							isDhcpWorking = true;
						}
					}
				}
				else {
						/* Trying to get an IP address */
						//ip = IPAddress(192, 168, 1, 36);

						Ethernet.begin(mac);
						//system("ifconfig eth0 192.168.1.36");  // fixed ip address to ease the telnet connection
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
  
  // For debugging purpose, we start telnet
  // Remove for production use
  system("telnetd -l /bin/sh");
#endif
  
  delay(3000);
  Serial.begin(9600);
  Serial.println("#############INITIALIZING DEVICE#############\n");

  /* Calibrating Accelerometer */
  accelero.begin(13, 12, 11, 10, A0, A1, A2);     // set the proper pin x y z
  accelero.setSensitivity(LOW);                  // sets the sensitivity to +/-6G
  accelero.calibrate();
  accelero.setAveraging(1);  // number of samples that have to be averaged
  
  #ifdef __IS_GALILEO
	  // Workaround for Galileo (and other boards with Linux)
	  system("/etc/init.d/networking restart");
  #endif

	Serial.println("Setting up ethernet connection");
	setupEthernet();

  //system("cat /etc/resolv.conf > /dev/ttyGS0 < /dev/ttyGS0");  // DEBUG
  
  Serial.println("Forcing config update...");
  initConfigUpdates();
  
  Serial.println("Syncing NTP...");
  initNTP();
  // We need to set this AFTER ntp sync...
  lastCfgUpdate = getUNIXTime();
  
  Serial.println("EEPROM init");
  initEEPROM();
  
  Serial.println("UDP Command Socket init");
  commandInterfaceInit();
  
  Serial.print("Free RAM: ");
  Serial.println(freeRam());
  
  Serial.println("\n#############INIZIALIZATION COMPLETE!#############");
}

void loop() {
  doNTPActions();
  doConfigUpdates();
  
  int cHour = (getUNIXTime() % 86400L) / 3600;
  if(currentHour != cHour)
  {
    currentHour = cHour;
    checkCalibrationNeeded(accelero, cHour);
  }
  
  checkCommandPacket();
  
  checkSensore();
}

