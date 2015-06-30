
#ifndef __GALILEO_CORE_CPP
#define __GALILEO_CORE_CPP

#include "buildcfg.h"

#include <math.h>
#include <sys/sysinfo.h>
#include <signal.h>
#include <stdlib.h>
#include <stdint.h>

#ifdef __IS_GALILEO
	#include "AcceleroMMA7361.h"
	#include <Arduino.h>

	#include <pins_arduino.h>
	#include <BitsAndBytes.h>
	#include <Ethernet.h>
	#include <SPI.h>
	#include <EEPROM.h>
	#include <SD.h>
	#include <EthernetUdp.h>

#endif

#include "config.h"
#include "avg.h"
#include "GalileoLog.h"

byte zz = 0;

bool resetEthernet = false;
unsigned long freeRam();

unsigned long previousMillis = 0;        // will store last time LED was updated
unsigned long previousMillisNTP = 0;     // will store last time LED was updated
unsigned long pingIntervalCheckCounter = 0;
unsigned long calibrationInterval = 60*60*1000;
unsigned long prevMillisCalibration = 0;
unsigned long TimeEvent = 60*1000;// 60 second per Event
unsigned long startRecord = 0;
unsigned long millis24h;
unsigned long lastRstMill = 0;
int resetnum = 0;
int numAlert = 0;
int reTareNum = 40;

bool chekInternet = false;
bool recordData = false;
byte errors_connection = 0;

//definition of Accelerometer object
AcceleroMMA7361 accelero;
int currentHour = -1;

#include "commons.h"
#include "ntp_alt.h"
#include "threshold.h"
#include "localstream.h"
#include "httpconn.h"
#include "cfgupdate.h"




//definition freeRam method
#ifdef __IS_GALILEO
unsigned long freeRam() {
	struct sysinfo sys_info;
	if (sysinfo(&sys_info) == 0) {
		return sys_info.freeram;
	} else {
		return 0;
	}
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

// return true if at least one of the axis is over the threshold
boolean isOverThresholdBasic(struct RECORD *db, struct TDEF *td) {
	return (db->valx > td->pthresx || db->valx < td->nthresx)
		   || (db->valy > td->pthresy || db->valy < td->nthresy)
		   || (db->valz > td->pthresz || db->valz < td->nthresz);
}

boolean isOverThresholdFixed(struct RECORD *db, struct TDEF *td) {
	return (abs(db->valx) > td->pthresx)
		   || (abs(db->valy) > td->pthresy)
		   || (abs(db->valz - GFORCE) > td->pthresz);
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


	switch(thresholdAlgorithm) {
		case Fixed:
			db->overThreshold = isOverThresholdFixed(db, &td);
			break;
		case Basic:
		default:
			db->overThreshold = isOverThresholdBasic(db, &td);
			break;
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

		Log::i(db->overThreshold?"overThreshold":"inEvent");
		if (ledON && !redLedStatus){
			digitalWrite(LED_RED,HIGH);
			redLedStatus = !redLedStatus;
		}
		if (internetConnected && testNoInternet && start) {// send value data if there is a connection
			//if (ledON) digitalWrite(LED_GREEN,HIGH);
			if(alert){
				inEvent = 1;
				milldelayTimeEvent = millis(); // timestamp in millis for Event Interval */

				httpSendAlert1(db, &td);
				numAlert++;
				//getConfigNew();// on testing

			}else{
				Log::i("IN EVENT - BUT NOT ADJUSTED SENDVALUES");
			}
		}
		else {
			Log::d("NOT CONNECTED - or not in start!");
		}
	}
	else {
		if (ledON && !(inEvent) && redLedStatus){
			digitalWrite(LED_RED,LOW);
			redLedStatus = !redLedStatus;
		}
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
		Log::d("Valori Accelerometro: %lf %lf %lf", db->valx, db->valy, db->valz);
	}
}

// set up the ethernet connection per location;
// use the linux underneath the device to force manual DNS and so on
void setupEthernet() {
	timeServer = IPAddress(178, 33, 50, 131);
	if (isDhcpEnabled) {
		boolean isDhcpWorking = false;
		while (!isDhcpWorking) {
			// WARNING: add DHCP timeout
			// Trying to get an IP address
			if (Ethernet.begin(mac) == 0) {
				// Error retrieving DHCP IP
				Log::e("Error while attempting to get an IP, retrying in 5 seconds...");
				delay(5000);
			} else {
				// DHCP IP retireved successfull
				char buf[300];
				IPAddress localIp = Ethernet.localIP();
				snprintf(buf, 300, "%i.%i.%i.%i", localIp[0], localIp[1], localIp[2], localIp[3]);
				Log::d("IP retrived successfully from DHCP: %s", buf);
				isDhcpWorking = true;
			}
		}
	} else {
		// Home Static Configuration
		Log::i("Static Configuration\n");
		//add your static IP here
		if (GEN1) ip = IPAddress(192, 168, 1, 177);// gen1
		else ip = IPAddress(192, 168, 1, 178); // gen2
		//dnsServer = IPAddress(192,168,1,1);
		dnsServer = IPAddress(192,168,1,254);
		//gateway = IPAddress(192,168,1,1);
		gateway = IPAddress(192,168,1,254);
		subnet = IPAddress(255, 255 ,255 ,0);
		// ARDUINO START CONNECTION
		Ethernet.begin(mac, ip, dnsServer, gateway, subnet); // Static address configuration
		//LINUX SYSTEM START CONNECTION
		if (GEN1) system("ifconfig eth0 192.168.1.177 netmask 255.255.255.0 up");  // set IP and SubnetMask for the Ethernet
		else system("ifconfig eth0 192.168.1.178 netmask 255.255.255.0 up");  // set IP and SubnetMask for the Ethernet
		//else system("ifconfig eth0 192.168.1.178 netmask 255.255.255.0 up > /dev/ttyGS0 < /dev/ttyGS0");  // set IP and SubnetMask for the Ethernet
		//system("route add default gw 192.168.1.254 eth0 > /dev/ttyGS0 < /dev/ttyGS0");  // change the Gateway for the Ethernet
		system("route add default gw 192.168.1.254 eth0");  // change the Gateway for the Ethernet
		system("echo 'nameserver 8.8.8.8' > /etc/resolv.conf");  // add the GOOGLE DNS
		//system("ifconfig eth0 192.168.1.36");  // fixed ip address to use the telnet connection
		//system("ifconfig > /dev/ttyGS0");  // debug
		char buf[300];
		IPAddress localIp = Ethernet.localIP();
		snprintf(buf, 300, "%i.%i.%i.%i", localIp[0], localIp[1], localIp[2], localIp[3]);
		Log::d("Static IP: %s");
	}
}

void setup() {
	Log::setLogFile(DEFAULT_LOG_PATH);

	analogReadResolution(10);        // 3.3V => 4096
	Log::i("    Res12bit: ");
	delay(1000);
	delay(500);
	Log::i("Starting.........");
#ifdef __IS_GALILEO
	// Fixing Arduino Galileo bug
	signal(SIGPIPE, SIG_IGN); // TODO: Remove? - caused not restarting sketch
	// Workaround for Galileo (and other boards with Linux)
	system("/etc/init.d/networking restart");
	delay(1000);
	// Remove for production use
	system("telnetd -l /bin/sh");
#endif
	//system("/etc/init.d/networking restart");
	//delay(1000);
	//system("telnetd -l /bin/sh");
	//storeConfigToSD();
	//delay(300);
	Log::i("INITIALIZING DEVICE");
	//if (logON) log("###INITIALIZING DEVICE###");
	Log::i("readConfig()");
	readConfig(); // read config from SD Card
	/* Calibrating Accelerometer */
	accelero.begin(/* 13, 12, 11, 10, */ A0, A1, A2);     // set the proper pin x y z
	//accelero.setSensitivity(LOW);                  // sets the sensitivity to +/-6G
	Log::d("calibrate()");
	accelero.setAveraging(10);  // number of samples that have to be averaged
	accelero.calibrate();
	Log::d("setAveraging(10)");
	// Config connection on Ethernet module
	Log::d("Setting up ethernet connection");
	setupEthernet();
	delay(1500);


	IPAddress syslogIp(192, 0, 2, 75);
	Log::setSyslogServer(syslogIp);
	Log::i("Syslog enabled");


	//byteMacToString(mac); // create string for MAC address
	if (request_mac_from_server) {
		Log::i("Requesting deviceID to server... ");
		getMacAddressFromServer(); // asking for new mac address/deviceid
		HEXtoDecimal(mac_string, strlen(mac_string), mac);
		byteMacToString(mac);
		// convertMACFromStringToByte();
		Log::setDeviceId(mac_string);
	}
	if(!request_lat_lon) start = true;

	internetConnected = isConnectedToInternet();
	delay(500); // wait internet connaction Status
	Log::i("STATUS CONNECTION: %s", internetConnected ? "CONNECTED" : "NOT CONNECTED");
	delay(500);

	if (ledON) {
		pinMode(LED_GREEN, OUTPUT);
		digitalWrite(LED_GREEN,HIGH);
		delay(500);
		digitalWrite(LED_GREEN,LOW);
		if (internetConnected){
			digitalWrite(LED_GREEN,HIGH);
			greenLedStatus = true;
		}else greenLedStatus = false;

		digitalWrite(LED_RED,HIGH);
		delay(500);
		digitalWrite(LED_RED,LOW);
		pinMode(LED_RED, OUTPUT);
		digitalWrite(LED_RED,LOW);
		redLedStatus = false;
		pinMode(LED_YELLOW, OUTPUT);
		digitalWrite(LED_YELLOW,HIGH);
		delay(500);
		digitalWrite(LED_YELLOW,LOW);
	}

	//system("cat /etc/resolv.conf > /dev/ttyGS0 < /dev/ttyGS0");  // DEBUG
	Log::d("Forcing config update...");
	initConfigUpdates();
	Log::d("EEPROM init");
	//initThrSD(false);
	//initEEPROM(forceInitEEPROM);
	Log::d("UDP Command Socket init");
	commandInterfaceInit(); // open port for mobile app

	if(internetConnected && testNoInternet){
		Log::d("Syncing NTP...");
		initNTP(); // controllare il ritorno ++++++++++++++++++++++++++++++++++++++++++++
		// We need to set this AFTER ntp sync...
		lastCfgUpdate = getUNIXTime();
	}

	if(doesFileExist(script_reset)!= 1){ // check if reset script exists
		Log::d("createScript...");
		createScript(script_reset, reboot_scriptText);
	}

	Log::d("Free RAM: %lu", freeRam());
	Log::d("INIZIALIZATION COMPLETE!");
	Log::d("Testttttttt - FINITOOOOO");

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
				digitalWrite(LED_GREEN,LOW);
				greenLedStatus = !greenLedStatus;
			}
			resetEthernet = true;
		}
		else { // IF INTERNET IS PRESENT
			if (ledON && !greenLedStatus){
				digitalWrite(LED_GREEN,HIGH);
				greenLedStatus = !greenLedStatus;
			}
			//doConfigUpdates(); // controllare +++++++++++++++++++++++++++++++++++++++++
			resetEthernet = false;
			// if (start)getConfigNew(); // CHEK FOR UPDATES

		}
		Log::d("Still running: %s CHECK INTERNET INTERVAL: %lu STATUS: %s",
			   getGalileoDate(), checkInternetConnectionInterval, internetConnected?"CONNECTED":"NOT CONNECTED");
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
		Log::d("checkCalibrationNeeded---------------------------------------");
		// checkCalibrationNeeded(accelero, cHour);
		checkCalibrationNeededNOSD(accelero, cHour);
		ForceCalibrationNeeded = false;
		prevMillisCalibration = currentMillis;
	}
	if(numAlert >= reTareNum){
		Log::i(" alert >40 recalibrating......");
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
			Log::e("networking restart - NOT CONNECTED FINTO!!!");
			// Workaround for Galileo (and other boards with Linux)
			system("/etc/init.d/networking restart");

			delay(2000);
			setupEthernet();
			delay(1000);
			lastRstMill = currentMillis;
			chekInternet = true;
			resetnum++;
			if (resetnum > 2){
				Log::d("SYSTEM restart!!!");
				digitalWrite(LED_RED, HIGH);
				digitalWrite(LED_GREEN, LOW);
				delay(500);
				digitalWrite(LED_RED, LOW);
				digitalWrite(LED_GREEN, HIGH);
				delay(500);
				digitalWrite(LED_RED, HIGH);
				digitalWrite(LED_GREEN, LOW);
				delay(500);
				digitalWrite(LED_RED, LOW);
				digitalWrite(LED_GREEN, HIGH);
				delay(1500);
				//execScript(script_reset);
				system("reboot");
				//system("shutdown -r -t sec 00");
				for(;;);
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
				Log::i("###############STARTING RECORD----------------------------------------");
			}
			debug_Axis();

			if (currentMillis - startRecord >= 3600000UL && zz == 1/* 3600000UL */){ // AFTER TIME RECORD
				zz = 2;
				Log::i("###############STOPPING RECORD----------------------------------------");
			}
		}
		milldelayTime = currentMillis;
	}

	// check if is time to update config
	if (/* start && */ testNoInternet && internetConnected){
		if (currentMillis - lastCfgUpdate > checkConfigInterval){
			getConfigNew(); // CHEK FOR UPDATES
			lastCfgUpdate = currentMillis;
			printConfig();
		}

	}

	// reset the device every 24h or after 20 errors
	if (((currentMillis - millis24h >=  86400000UL * 2  ) && !inEvent /* && !inEventSD */) || (errors_connection > 20)){
		Log::i("Reboot after 24h of activity");
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

#endif
