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
#include "CommandInterface.h"
#include "generic.h"
#include "net/SCSAPI_MQTT.h"
#include "net/SCSAPI_HTTP.h"

#ifndef NOWATCHDOG

#include "Watchdog.h"

#endif

Seismometer *seismometer;
SCSAPI *scsapi;
bool networkConnected = false;
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

void apiInit();

void handleNetworkError(bool cstatus);

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
		Config::setMacAddress("000000000000");
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
//	HTTPClient::sendCrashReports();
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
	apiInit();
	LED::green(false);
	LED::yellow(false);
	LED::setLedAnimation(true);

	Log::i("Time sync");
	// NTP SYNC with NTP server
	scsapi->requestTimeUpdate();
	unsigned long ntpstartms = Utils::millis();
	while (getUNIXTime() == 0) {
		scsapi->tick();
		Utils::delay(50);
#ifndef NOWATCHDOG
		Watchdog::heartBeat();
#endif

		if (getUNIXTime() == 0 && (Utils::millis() - ntpstartms) > (30 * 1000)) {
			Log::e("Unable to receive NTP from server, rebooting");
			platformReboot();
		}
	}
	Log::i("Time sync'ed");

	Log::i("Starting UDP local command interface");
	CommandInterface::commandInterfaceInit();

	Log::d("Free RAM: %lu", Utils::getFreeRam());

	Log::d("Sending Alive");
	handleNetworkError(scsapi->alive());
	cfgLastMs = Utils::millis();

	Log::d("INIZIALIZATION COMPLETE!");

	LED::setLedAnimation(false);
	LED::startupBlink();
	LED::green(true);
}

void apiInit() {
	unsigned long connstartms = Utils::millis();
	while (!networkConnected) {
		scsapi = new SCSAPI_MQTT();
		if (!scsapi->init()) {
			Log::e("Error connecting to MQTT server, retrying with HTTP");
			scsapi = new SCSAPI_HTTP();
			if (!scsapi->init()) {
				Log::e("Error connecting to HTTP server");
			} else {
				networkConnected = true;
			}
		} else {
			networkConnected = true;
		}
		if (!networkConnected && Utils::millis() - connstartms > 60 * 60 * 1000) {
			Log::e("Unable to connect to server in 1h, rebooting");
			platformReboot();
		}
		if (!networkConnected) {
			LED::setLedAnimation(false);
			LED::green(false);
			LED::yellow(true);
			Log::e("Retrying in 5 seconds");
			Utils::delay(5 * 1000);
		}
#ifndef NOWATCHDOG
		Watchdog::heartBeat();
#endif
	}
}

void handleNetworkError(bool cstatus) {
	networkConnected = cstatus;
	LED::yellow(!networkConnected);
	if (!cstatus) {
		apiInit();
	}
}

void loop() {
	LED::tick();
	scsapi->tick();

#ifndef NOWATCHDOG
	Watchdog::heartBeat();
#endif

	CommandInterface::checkCommandPacket();

	if (Utils::millis() - netLastMs >= CHECK_NETWORK_INTERVAL) {
		// If no MAC Address detect we presume that ethernet interface is down, so we'll reboot
		std::string macAddress = Utils::getInterfaceMAC();
		if (macAddress.empty()) {
			Log::e("Unable to get MAC Address, rebooting");
			platformReboot();
		}

		// TODO handle disconnection
		handleNetworkError(scsapi->ping());

		netLastMs = Utils::millis();
	}

	if (Utils::millis() - cfgLastMs >= ALIVE_INTERVAL) {
		handleNetworkError(scsapi->alive());
		cfgLastMs = Utils::millis();
	}

	if (Utils::millis() - ntpLastMs >= NTP_SYNC_INTERVAL) {
		handleNetworkError(scsapi->requestTimeUpdate());
		ntpLastMs = Utils::millis();
	}

	if (Utils::millis() - seismoLastMs >= SEISMOMETER_TICK_INTERVAL) {
		seismometer->tick(scsapi);
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
