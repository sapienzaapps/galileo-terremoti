//
// Created by ebassetti on 21/08/15.
//

#include "../../generic.h"
#include "../../Log.h"
#include "ADXL345Accelerometer.h"
#include "../../Utils.h"
#include "../../LED.h"
#include <string.h>

std::string executablePath = "";

void vendor_init(int argc, char** argv) {
	if(argv != NULL) {
		Log::d("vendor_init called with %i arguments", argc);

		char executableBuf[1024];
		memset(executableBuf, 0, 1024);
		readlink("/proc/self/exe", executableBuf, 1023);
		executablePath = std::string(executableBuf);
	}
}

Accelerometer* getAccelerometer() {
	return new ADXL345Accelerometer(0x53);
}

std::string getPlatformName() {
	return "RaspberryPI";
}

void platformReboot() {
	Log::d("Rebooting");
	Log::close();
	system("reboot");
	while(true) {};
}

void platformUpgrade(std::string path) {
	LED::green(false);
	LED::yellow(false);
	LED::red(false);
	LED::setLedBlinking(LED_RED_PIN);

	Log::d("Download sketch upgrade: %s", path.c_str());

	char cmd[1024];
	memset(cmd, 0, 1024);
	snprintf(cmd, 1023, "curl -o /tmp/sketch.new %s", path.c_str());
	system(cmd);

	memset(cmd, 0, 1024);
	snprintf(cmd, 1023, "mv /tmp/sketch.new %s", executablePath.c_str());
	system(cmd);

	memset(cmd, 0, 1024);
	snprintf(cmd, 1023, "chmod a+x %s", executablePath.c_str());
	system(cmd);

	if(Utils::fileExists("/bin/systemctl")) {
		Log::d("Systemctl detected");
		// Assuming that we're on a systemd installation (eg. Raspbian Jessie)
		int retval = system("systemctl restart galileo-terremoti");
		if(retval != 0) {
			Log::d("Systemctl restart error, trying the hard way");
			// TODO: we can do better than that...
			platformReboot();
		}
		Log::close();
		while(true) {};
	} else {
		// Old methods - also known as "Windows-style upgrades"
		// TODO: check for init.d script and use it if exists instead of rebooting
		platformReboot();
	}
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
