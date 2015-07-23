#ifndef __GALILEO_CORE_CPP
#define __GALILEO_CORE_CPP

#include <stdlib.h>
#include <stdint.h>

#include "common.h"
#include "Config.h"
#include "Log.h"
#include "Seismometer.h"
#include "LED.h"
#include "Utils.h"
#include "net/NTP.h"
#include "net/NetworkManager.h"

Seismometer seismometer;

void setup();
void loop();

int main() {
	setup();
	while(1) {
		loop();
	}
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
	// TODO: start command interface

	if(!Config::hasPosition()) {
		Log::i("Getting position information");
		// TODO: get position if not avail / or wait for location from App if not avail
	} else {
		Log::i("GPS coords: %f %f", Config::getLatitude(), Config::getLongitude());
	}

	Log::i("Init seismometer");
	seismometer.init();

	Log::d("Free RAM: %lu", Utils::freeRam());
	Log::d("INIZIALIZATION COMPLETE!");

	LED::startupBlink();
}

void loop() {

}

#endif
