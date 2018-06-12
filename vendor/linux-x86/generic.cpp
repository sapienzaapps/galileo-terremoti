//
// Created by ebassetti on 21/08/15.
//

#include "../../generic.h"
#include "../../Log.h"
#include "DummyAccelerometer.h"
#include "../../Utils.h"
#ifdef SDL_DEMO
#include "../../LCDMonitor.h"
#endif

void vendor_init(int argc, char** argv) {
	if(argv != NULL) {
		Log::d("vendor_init called with %i arguments", argc);
	}
#ifdef SDL_DEMO
	LCDMonitor::getInstance();
#endif
}

Accelerometer* getAccelerometer() {
	return new DummyAccelerometer();
}

std::string getPlatformName() {
	return "Linux Generic";
}

void platformReboot() {
	exit(0);
}

void platformUpgrade(std::string path) {
	Log::d("Received upgrade hint for %s", path.c_str());
}

unsigned long lastNTPTime = 0;
unsigned long lastNTPMillis = 0;

// Set date and time to NTP's retrieved one
void execSystemTimeUpdate(time_t epoch) {
	lastNTPMillis = Utils::millis();
	lastNTPTime = epoch;
}

unsigned long getUNIXTime() {
	if (lastNTPMillis == 0) {
		return 0;
	}
	unsigned long diff = Utils::millis() - lastNTPMillis;
	return (lastNTPTime + (diff / 1000));
}

