#ifndef __GALILEO_CORE_CPP
#define __GALILEO_CORE_CPP

#include <stdlib.h>
#include <stdint.h>
#include <execinfo.h>
#include <sys/fcntl.h>
#include <string.h>

#include "common.h"
#include "Config.h"
#include "Log.h"
#include "LED.h"
#include "Utils.h"
#include "net/NTP.h"
#include "net/NetworkManager.h"
#include "CommandInterface.h"
#include "generic.h"
#include "net/HTTPClient.h"

#ifndef NOWATCHDOG

#include "Watchdog.h"
#include "net/SCSAPI.h"

#endif

Seismometer *seismometer;
unsigned long netLastMs = 0;
unsigned long ntpLastMs = 0;
unsigned long cfgLastMs = 0;
unsigned long seismoLastMs = 0;
unsigned long logRotationMs = 0;
#ifdef DEBUG
unsigned long valgrindMs = 0;
#endif

void setup();

void loop();

#ifdef DEBUG

void crashHandler(int sig) {
	void *array[10];

	// get void*'s for all entries on the stack
	int size = backtrace(array, 10);

	// print out all the frames to stderr
	fprintf(stderr, "Error: signal %d:\n", sig);
	backtrace_symbols_fd(array, size, STDERR_FILENO);

	int fd = open(STACKTRACEINFO, O_RDWR | O_TRUNC);
	if (fd < 0) {
		// Ooops! Cannot create stack trace info
		exit(1);
	}
	std::string sigError = "Error: signal " + Utils::toString(sig);
	write(fd, sigError.c_str(), sigError.length());
	backtrace_symbols_fd(array, size, fd);
	close(fd);

	exit(1);
}

#endif

int main(int argc, char **argv) {
	vendor_init(argc, argv);

#ifdef DEBUG
	if (argc > 1 && strcmp("--valgrind", argv[1]) == 0) {
		valgrindMs = Utils::millis();
		HTTPClient::setBaseURL("http://192.0.2.20/seismocloud/");
		Config::setMacAddress("000000000000");
		Config::setLatitude(0.1);
		Config::setLongitude(0.1);
	} else if (argc > 1 && strcmp("--raw", argv[1]) == 0) {
		Accelerometer *accel = getAccelerometer();

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"
		while (true) {
			if (Utils::millis() - seismoLastMs >= SEISMOMETER_TICK_INTERVAL) {

				double x = accel->getXAccel();
				double y = accel->getYAccel();
				double z = accel->getZAccel();

				printf("%i - %f - %f - %f\n", Utils::millis(), x, y, z);

				seismoLastMs = Utils::millis();
			}
		}
#pragma clang diagnostic pop
	} else {
		signal(SIGSEGV, crashHandler);
	}
#endif

#ifndef NOWATCHDOG
	Watchdog::launch();
#endif

	setup();

#ifdef DEBUG
	HTTPClient::sendCrashReports();
#endif
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"
	while (1) {
		loop();
#ifndef GALILEO_GEN
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
#ifdef DEBUG
	Log::enableStdoutDebug(true);
	Log::setLogLevel(LEVEL_DEBUG);
#endif

	Log::i("Starting.........");

	// If no MAC Address detect we presume that ethernet interface is down, so we'll reboot
	std::string macAddress;
	int i = 0;
	do {
		macAddress = Utils::getInterfaceMAC();
		if (i >= 20) {
			Log::e("MAC Address primitive failed and timeout, rebooting");
			platformReboot();
		}
		if (macAddress.empty()) {
			Log::e("MAC Address primitive failed, retrying");
			Utils::delay(1000 * 3);
			i++;
		}
	} while (macAddress.empty());

	Log::i("Loading config");
	// Load saved config - if not available, load defaults
	Config::init();

	Config::printConfig();

	Log::i("Network init");
	// Network init
	NetworkManager::init();

	if (!Config::hasMACAddress()) {
		Log::i("Using default MAC Address: %s", macAddress.c_str());
		Config::setMacAddress(macAddress);
	} else {
		Log::i("Configured MAC Address: %s", Config::getMacAddress().c_str());
	}

	Log::i("Init seismometer");
	seismometer = Seismometer::getInstance();
	seismometer->init();

	Log::i("API connect");
	// Download new config from server
	if (!SCSAPI::init()) {
		Log::e("Error connecting to server");
		Utils::delay(5 * 1000);
		platformReboot();
	}

	Log::i("NTP sync");
	// NTP SYNC with NTP server
	NTP::init();
	NTP::sync();

	Log::i("Starting UDP local command interface");
	CommandInterface::commandInterfaceInit();

	Log::d("Free RAM: %lu", Utils::getFreeRam());

	Log::d("Sending Alive");
	SCSAPI::alive();

	Log::d("INIZIALIZATION COMPLETE!");

	LED::setLedAnimation(false);
	LED::startupBlink();
	LED::green(true);
}

void loop() {
	LED::tick();
#ifndef NOWATCHDOG
	Watchdog::heartBeat();
#endif

	CommandInterface::checkCommandPacket();

	if (Utils::millis() - netLastMs >= CHECK_NETWORK_INTERVAL) {

		// If no MAC Address detect we presume that ethernet interface is down, so we'll reboot
		std::string macAddress = Utils::getInterfaceMAC();
		if (macAddress.empty()) {
			platformReboot();
		}

		LED::yellow(!NetworkManager::isConnectedToInternet(true));
		netLastMs = Utils::millis();
	}

	if (Utils::millis() - cfgLastMs >= CHECK_CONFIG_INTERVAL) {
		Config::checkServerConfig();
		SCSAPI::alive();
		cfgLastMs = Utils::millis();
	}

	if (Utils::millis() - ntpLastMs >= NTP_SYNC_INTERVAL) {
		while (!NTP::sync());
		ntpLastMs = Utils::millis();
	}

	if (Utils::millis() - seismoLastMs >= SEISMOMETER_TICK_INTERVAL) {
		seismometer->tick();
		seismoLastMs = Utils::millis();
	}

	if (Utils::millis() - logRotationMs >= 1000 * 60 * 60 * 24) {
		Log::rotate();
	}

#ifdef DEBUG
	if (valgrindMs > 0 && Utils::millis() - valgrindMs > 1000 * 60 * 1) {
		exit(EXIT_SUCCESS);
	}
#endif
}

#endif
