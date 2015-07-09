#ifndef __GALILEO_CORE_CPP
#define __GALILEO_CORE_CPP

#include "buildcfg.h"

#include <sys/sysinfo.h>
#include <signal.h>
#include <stdlib.h>
#include <stdint.h>

#ifdef __IS_GALILEO
	#include "AcceleroMMA7361.h"

#include <Ethernet.h>

#endif

#include "config.h"
#include "avg.h"
#include "GalileoLog.h"

byte zz = 0;

unsigned long freeRam();

unsigned long previousMillis = 0;        // will store last time LED was updated
unsigned long previousMillisNTP = 0;     // will store last time LED was updated
unsigned long calibrationInterval = 60*60*1000;
unsigned long prevMillisCalibration = 0;
unsigned long startRecord = 0;
unsigned long millis24h;
int numAlert = 0;
int reTareNum = 40;
byte inEvent = 0;

bool recordData = false;
byte errors_connection = 0;

bool connectedToInternet = false;

//definition of Accelerometer object
AcceleroMMA7361 accelero;

#include "commons.h"
#include "ntp_alt.h"
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
bool isOverThresholdBasic(struct RECORD *db, struct TDEF *td) {
	return (db->valx > td->pthresx || db->valx < td->nthresx)
		   || (db->valy > td->pthresy || db->valy < td->nthresy)
		   || (db->valz > td->pthresz || db->valz < td->nthresz);
}

bool isOverThresholdFixed(struct RECORD *db, struct TDEF *td) {
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
		db->ms = NTP::getUNIXTimeMS();

		Log::i(db->overThreshold?"overThreshold":"inEvent");
		if (ledON && !redLedStatus){
			digitalWrite(LED_RED,HIGH);
			redLedStatus = !redLedStatus;
		}
		if (connectedToInternet && start) {// send value data if there is a connection
			//if (ledON) digitalWrite(LED_GREEN,HIGH);
			if(alert){
				inEvent = 1;
				milldelayTimeEvent = millis(); // timestamp in millis for Event Interval */

				HTTPClient::httpSendAlert1(db, &td);
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

	byte mac[6];
	Config::getMacAddressAsByte(mac);

	if (DHCP_CLIENT_ENABLED) {
		NetworkManager::setupAsDHCPClient(mac);
	} else {
		IPAddress staticAddress(192, 168, 1, 5);
		IPAddress subnetMask(255, 255, 255, 0);
		IPAddress gateway(192, 168, 1, 1);
		IPAddress dnsServer(8, 8, 8, 8);
		NetworkManager::setupStatic(mac, staticAddress, subnetMask, gateway, dnsServer);
	}
}

void setup() {
	Log::setLogFile(DEFAULT_LOG_PATH);
	Log::setLogLevel(LEVEL_DEBUG);

	analogReadResolution(10);        // 3.3V => 4096

	Log::i("Starting.........");

#ifdef __IS_GALILEO
	Log::i("Fix Galileo bugs");
	// Fixing Arduino Galileo bug
	signal(SIGPIPE, SIG_IGN); // TODO: Remove? - caused not restarting sketch
	// Workaround for Galileo (and other boards with Linux)
	system("/etc/init.d/networking restart");
	delay(1000);
	// Remove for production use
	//system("telnetd -l /bin/sh");
#endif

	Log::i("Loading config...");
	Config::readConfigFile(DEFAULT_CONFIG_PATH);

	Log::i("Initial calibration");
	/* Calibrating Accelerometer */
	accelero.begin(/* 13, 12, 11, 10, */ A0, A1, A2);     // set the proper pin x y z
	accelero.setAveraging(10);  // number of samples that have to be averaged
	accelero.calibrate();

	Log::d("calibration ended, starting up networking");

	setupEthernet();
	if(!NetworkManager::isConnectedToInternet(true)) {
		NetworkManager::forceRestart();
	}

	IPAddress syslogIp(192, 0, 2, 71);
	Log::setSyslogServer(syslogIp);
	Log::i("Syslog enabled");

	if (!Config::hasMACAddress()) {
		Log::i("Requesting deviceID to server... ");
		std::string mac_string;
		do {
			mac_string = HTTPClient::getMACAddress(); // asking for new mac address/deviceid
			if (mac_string == "") {
				Log::e("Cannot get MAC Address from server, retrying in 5 seconds...");
				sleep(5000);
			}
		} while(mac_string == "");
		Config::setMacAddress(mac_string);
		Log::setDeviceId(mac_string);
	}
	if(Config::hasPosition()) {
		start = true;
	}

	bool internetConnected = NetworkManager::isConnectedToInternet();

	Log::i("STATUS CONNECTION: %s", internetConnected ? "CONNECTED" : "NOT CONNECTED");

	if (ledON) {
		pinMode(LED_GREEN, OUTPUT);
		pinMode(LED_YELLOW, OUTPUT);
		pinMode(LED_RED, OUTPUT);

		digitalWrite(LED_GREEN,HIGH);
		delay(500);
		digitalWrite(LED_GREEN,LOW);
		if (internetConnected) {
			digitalWrite(LED_GREEN,HIGH);
			greenLedStatus = true;
		} else {
			greenLedStatus = false;
		}

		digitalWrite(LED_RED,HIGH);
		delay(500);
		digitalWrite(LED_RED,LOW);
		digitalWrite(LED_RED,LOW);
		redLedStatus = false;

		digitalWrite(LED_YELLOW,HIGH);
		delay(500);
		digitalWrite(LED_YELLOW,LOW);
	}

	Log::d("Forcing config update...");
	initConfigUpdates();

	Log::d("UDP Command Socket init");
	commandInterfaceInit();

	if(internetConnected){
		Log::d("Syncing NTP...");
		NTP::initNTP(); // controllare il ritorno ++++++++++++++++++++++++++++++++++++++++++++
		// We need to set this AFTER ntp sync...
		lastCfgUpdate = NTP::getUNIXTime();
	}

	if(doesFileExist(script_reset) != 1) { // check if reset script exists
		Log::d("createScript...");
		createScript(script_reset, reboot_scriptText);
	}

	Log::d("Free RAM: %lu", freeRam());
	Log::d("INIZIALIZATION COMPLETE!");

	millis24h =  millis();
	milldelayTime = millis24h;
}
// end SETUP

void loop() {
	currentMillis = millis();

	if ((currentMillis - previousMillis > checkInternetConnectionInterval)) {
		connectedToInternet = NetworkManager::isConnectedToInternet(true);
		if(!connectedToInternet) {
			NetworkManager::restart();
		}

		if (!connectedToInternet) {
			if (ledON && greenLedStatus){
				digitalWrite(LED_GREEN,LOW);
				greenLedStatus = !greenLedStatus;
			}
		} else {
			if (ledON && !greenLedStatus){
				digitalWrite(LED_GREEN,HIGH);
				greenLedStatus = !greenLedStatus;
			}
		}
		Log::d("Still running: %s CHECK INTERNET INTERVAL: %lu STATUS: %s",
			   getGalileoDate(), checkInternetConnectionInterval, connectedToInternet?"CONNECTED":"NOT CONNECTED");
		previousMillis = currentMillis;
	}

	// sync with the NTP server
	if ((currentMillis - previousMillisNTP > NTPInterval) && connectedToInternet) {
		NTP::dataPacket();
		previousMillisNTP = currentMillis;
	}

	// Check for calibration Sensor
	// unsigned long currentMillisCalibration = millis();
	if ((currentMillis - prevMillisCalibration > calibrationInterval) || ForceCalibrationNeeded) {
		int cHour = (int)(NTP::getUNIXTime() % 86400L) / 3600;
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
		int cHour = (int)(NTP::getUNIXTime() % 86400L) / 3600;
		checkCalibrationNeededNOSD(accelero, cHour);
		numAlert = 0;
	}
	if (inEvent) {// unlock inEvent time
		// unsigned long millisTimeEvent = millis();
		if (currentMillis - milldelayTimeEvent > HTTPClient::getNextContact() /* TimeEvent */) { // unlock Alert after xxx millisecs
			inEvent = 0;
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
	if (/* start && */ connectedToInternet){
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
