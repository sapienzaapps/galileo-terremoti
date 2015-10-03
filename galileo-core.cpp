#ifndef __GALILEO_CORE_CPP
#define __GALILEO_CORE_CPP

#include <stdlib.h>
#include <stdint.h>

#include "common.h"
#include "Config.h"
#include "Log.h"
#include "LED.h"
#include "Utils.h"
#include "net/NTP.h"
#include "net/NetworkManager.h"
#include "CommandInterface.h"
#include "Watchdog.h"
#include "generic.h"

Seismometer *seismometer;
unsigned long netLastMs = 0;
unsigned long ntpLastMs = 0;
unsigned long cfgLastMs = 0;
unsigned long seismoLastMs = 0;
unsigned long logRotationMs = 0;

void setup();
void loop();

int main(int argc, char** argv) {
	vendor_init(argc, argv);

	Watchdog::launch();
	setup();
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"
	while(1) {
		loop();
#ifndef __IS_GALILEO
		Utils::delay(50);
#endif
	}
#pragma clang diagnostic pop
	return EXIT_SUCCESS;
}

void setup() {
	logRotationMs = Utils::millis();
	LED::init(LED_GREEN_PIN, LED_YELLOW_PIN, LED_RED_PIN);
	LED::setLedAnimation(true);

	Log::setLogFile(DEFAULT_LOG_PATH);
	Log::enableStdoutDebug(true);
	Log::setLogLevel(LEVEL_DEBUG);

	Log::i("Starting.........");

	Log::i("Software version: %s", SOFTWARE_VERSION);
	Log::i("Platform name: %s", PLATFORM);
#ifdef __IS_GALILEO
	Log::i("Galileo Gen: %i", GALILEO_GEN);
#endif

	Log::i("Loading config");
	// Load saved config - if not available, load defaults
	Config::init();

	Log::i("Network init");
	// Network init
	NetworkManager::init();

	if(!Config::hasMACAddress()) {
		std::string macAddress = Utils::getInterfaceMAC();
		if(!macAddress.empty()) {
			Log::i("Using default MAC Address: %s", macAddress.c_str());
			Config::setMacAddress(macAddress);
		} else {
			Log::e("Cannot detect MAC Address");
		}
	} else {
		Log::i("Configured MAC Address: %s", Config::getMacAddress().c_str());
	}

	Log::i("Check new config");
	// Download new config from server
	while(!Config::checkServerConfig()) {
		Log::e("Error checking server config");
		Utils::delay(5*1000);
	}

	Log::i("Update logging settings from config");
	// Re-init logging from config
	Log::updateFromConfig();

	Log::i("NTP sync");
	// NTP SYNC with NTP server
	NTP::init();
	NTP::sync();

	Log::i("Starting UDP local command interface");
	CommandInterface::commandInterfaceInit();

	if(!Config::hasPosition()) {
		Log::i("Getting position information");
		// TODO: Get position if not avail
		// Wait for location from App if not avail
		Log::i("Position not available, waiting for position from App");
		LED::setLedAnimation(false);
		LED::setLedBlinking(LED_YELLOW_PIN);
		do {
			Watchdog::heartBeat();
			CommandInterface::checkCommandPacket();
			Utils::delay(200);
		} while(!Config::hasPosition());
		LED::clearLedBlinking();
		LED::setLedAnimation(true);
	} else {
		Log::i("GPS coords: %f %f", Config::getLatitude(), Config::getLongitude());
	}

	Log::i("Init seismometer");
	seismometer = Seismometer::getInstance();
	seismometer->init();

	Log::d("Free RAM: %lu", Utils::freeRam());
	Log::d("INIZIALIZATION COMPLETE!");

	LED::setLedAnimation(false);
	LED::startupBlink();
	LED::green(true);
}

void loop() {
	LED::tick();
	Watchdog::heartBeat();

	CommandInterface::checkCommandPacket();

	if(Utils::millis() - netLastMs >= CHECK_NETWORK_INTERVAL) {
		LED::yellow(!NetworkManager::isConnectedToInternet(true));
		netLastMs = Utils::millis();
	}

	if(Utils::millis() - cfgLastMs >= CHECK_CONFIG_INTERVAL) {
		Config::checkServerConfig();
		cfgLastMs = Utils::millis();
	}

	if(Utils::millis() - ntpLastMs >= NTP_SYNC_INTERVAL) {
		while(!NTP::sync());
		ntpLastMs = Utils::millis();
	}

	seismometer->calibrateIfNeeded();

	if(Utils::millis() - seismoLastMs >= SEISMOMETER_TICK_INTERVAL) {
		seismometer->tick();
		seismoLastMs = Utils::millis();
	}

	if(Utils::millis() - logRotationMs >= 1000 * 60 * 60 * 24) {
		Log::rotate();
	}
}

#endif
