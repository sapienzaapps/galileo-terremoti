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

Seismometer seismometer;

void setup();
void loop();

int main() {
	setup();
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"
	while(1) {
		loop();
	}
#pragma clang diagnostic pop
	return EXIT_SUCCESS;
}

void setup() {
	LED::init(LED_GREEN_PIN, LED_YELLOW_PIN, LED_RED_PIN);
	Log::setLogFile(DEFAULT_LOG_PATH);
	Log::setLogLevel(LEVEL_INFO);

	Log::i("Starting.........");

	Log::i("Loading config");
	// Load saved config - if not available, load defaults
	Config::init();

	Log::i("Network init");
	// Network init
	NetworkManager::init();

	Log::i("Check new config");
	// Download new config from server
	Config::checkServerConfig();

	Log::i("Update logging settings from config");
	// Re-init logging from config
	Log::updateFromConfig();

	Log::i("NTP sync");
	// NTP SYNC with NTP server
	NTP::sync();

	Log::i("Starting UDP local command interface");
	CommandInterface::commandInterfaceInit();

	if(!Config::hasPosition()) {
		Log::i("Getting position information");
		// TODO: Get position if not avail
		// Wait for location from App if not avail
		Log::i("Position not available, waiting for position from App");
		do {
			CommandInterface::checkCommandPacket();
			delay(200);
		} while(!Config::hasPosition());
	} else {
		Log::i("GPS coords: %f %f", Config::getLatitude(), Config::getLongitude());
	}

	Log::i("Init seismometer");
	seismometer.init();

	Log::d("Free RAM: %lu", Utils::freeRam());
	Log::d("INIZIALIZATION COMPLETE!");

	LED::startupBlink();
}

unsigned long netLastMs = 0;
unsigned long ntpLastMs = 0;
unsigned long cfgLastMs = 0;
unsigned long seismoLastMs = 0;

void loop() {
	if(millis() - netLastMs >= CHECK_NETWORK_INTERVAL) {
		if(!NetworkManager::isConnectedToInternet(true)) {
			//NetworkManager::forceRestart();
		}
		netLastMs = millis();
	}

	if(millis() - cfgLastMs >= CHECK_CONFIG_INTERVAL) {
		Config::checkServerConfig();
		cfgLastMs = millis();
	}

	if(millis() - ntpLastMs >= NTP_SYNC_INTERVAL) {
		while(!NTP::sync());
		ntpLastMs = millis();
	}

	seismometer.calibrateIfNeeded();

	if(millis() - seismoLastMs >= SEISMOMETER_TICK_INTERVAL) {
		seismometer.tick();
		seismoLastMs = millis();
	}
}

#endif
