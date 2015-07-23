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

#include "Config.h"
#include "avg.h"
#include "Log.h"
#include "Seismometer.h"

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

byte errors_connection = 0;

bool connectedToInternet = false;

#include "commons.h"
#include "NTP.h"
#include "localstream.h"
#include "HTTPClient.h"
#include "cfgupdate.h"
#include "LED.h"

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

unsigned long currentMillis, milldelayTime;
#else
#if (ARDUINO >= 100)
  #include <Arduino.h>
#else
  #include <WProgram.h>
#endif
#endif

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
	Log::setLogLevel(LEVEL_INFO);

	Log::i("Starting.........");

	// Settings resolution to 10 => 3.3V => 4096
	analogReadResolution(10);

	Log::i("Loading config");
	// Load saved config - if not available, load defaults
	Config::init();

	Log::i("Network init");
	// Network init
	NetworkManager::init();

	Log::i("Check new config");
	// Download new config from server
	Config::checkServerConfig();

	Log::i("NTP sync");
	// SYNC with NTP server
	bool d;
	do {
		d = NTP::sync();
	} while(!d);

	Log::i("Update logging settings from config");
	// Re-init logging from config
	Log::updateFromConfig();

	Log::i("Init seismometer");
	Seismometer::init();






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



	LED::prepare(LED_GREEN, LED_MODE_OUTPUT);
	LED::prepare(LED_YELLOW, LED_MODE_OUTPUT);
	LED::prepare(LED_RED, LED_MODE_OUTPUT);

	LED::set(LED_GREEN, LED_ON);
	delay(500);
	LED::set(LED_GREEN, LED_OFF);

	if (internetConnected) {
		LED::set(LED_GREEN, LED_ON);
		greenLedStatus = true;
	} else {
		greenLedStatus = false;
	}

	LED::set(LED_RED, LED_ON);
	delay(500);
	LED::set(LED_RED, LED_OFF);
	redLedStatus = false;

	LED::set(LED_YELLOW, LED_ON);
	delay(500);
	LED::set(LED_YELLOW, LED_OFF);



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
		Log::d("Still running, CHECK INTERNET INTERVAL: %lu STATUS: %s",
			   checkInternetConnectionInterval, connectedToInternet?"CONNECTED":"NOT CONNECTED");
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
		forceInitCalibration = true;
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
		// check mobile command
		checkCommandPacket();
		// read sensor values
		checkSensore();
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
		for(;;){}
	}
}

#endif
