//
// Created by ebassetti on 21/08/15.
//

#include "../../generic.h"
#include "../../Log.h"
#include "ADXL345Accelerometer.h"
#include "../../Utils.h"
#include "../../LED.h"

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
	snprintf(cmd, 1023, "mv /tmp/sketch.elf %s", executablePath.c_str());
	system(cmd);

	if(Utils::fileExists("/bin/systemctl")) {
		// Assuming that we're on a systemd installation (eg. Raspbian Jessie)
		int retval = system("systemctl galileo-terremoti restart");
		if(retval != 0) {
			// TODO: we can do better than that...
			platformReboot();
		}
		while(true) {};
	} else {
		// Old methods - also known as "Windows-style upgrades"
		// TODO: check for init.d script and use it if exists instead of rebooting
		platformReboot();
	}
}