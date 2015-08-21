
#include <stdio.h>
#include <string.h>
#include <time.h>
// #include <Serial.h>
#include "common.h"
#include "Log.h"
#include "Config.h"

IPaddr Log::syslogServer(0, 0, 0, 0);
bool Log::syslogEnabled = false;
Udp Log::syslogUdp;
FILE *Log::logFile = NULL;
bool Log::serialDebug = true;
LogLevel Log::logLevel = LEVEL_INFO;
std::string Log::deviceid = "";
bool Log::stdoutDebug = false;

void Log::setDeviceId(std::string deviceid) {
	Log::deviceid = deviceid;
}

void Log::setSyslogServer(IPaddr server) {
	Log::syslogServer = server;
	Log::syslogEnabled = true;
	Log::syslogUdp.connectTo(Log::syslogServer, 514);
	// TODO: choose a special value to disable... maybe 0?
	//Log::syslogUdp.begin(514);
}

void Log::setLogFile(const char *filepath) {
	if (Log::logFile != NULL) {
		fclose(Log::logFile);
	}

	Log::logFile = fopen(filepath, "w");
	if (Log::logFile == NULL) {
		//Serial.println("Error opening log file\n");
	}
}

void Log::enableSerialDebug(bool serialDebug) {
	if (!Log::serialDebug && serialDebug) {
		//Serial.begin(9600);
	} else if (Log::serialDebug && !serialDebug) {
		//Serial.end();
	}
	Log::serialDebug = serialDebug;
}

void Log::log(LogLevel level, const char *msg, va_list argptr) {
	// Skip messages with low log priority
	if (Log::logLevel > level) {
		return;
	}

	// Preparing log message with arguments
	char realmsg[1024];
	vsnprintf(realmsg, 1024, msg, argptr);

	//Prepending log message with datetime and level
	char logentry[1024];
	char levelC = 'D';
	if (level == LEVEL_INFO) {
		levelC = 'I';
	} else if (level == LEVEL_ERROR) {
		levelC = 'E';
	}
	if (deviceid == "") {
		snprintf(logentry, 1024, "[%s] [%c] [?] %s", Log::getDateTime().c_str(), levelC, realmsg);
	} else {
		snprintf(logentry, 1024, "[%s] [%c] [%s] %s", Log::getDateTime().c_str(), levelC, deviceid.c_str(), realmsg);
	}

	if (Log::stdoutDebug) {
		printf("%s\n", logentry);
	}

	if (Log::serialDebug) {
		//Serial.println(logentry);
	}

	if (Log::syslogEnabled) {
		// priority = (facility * 8) + severity
		int priority = (16 * 8);
		if (level == LEVEL_INFO) {
			priority += 6;
		} else if (level == LEVEL_ERROR) {
			priority += 2;
		} else {
			priority += 7;
		}

		// 1024 is the maximum size of syslog UDP entry
		char pkt[1024];
		snprintf(pkt, 1024, "<%i>%s", priority, logentry);

		Log::syslogUdp.send(pkt, strlen(pkt), Log::syslogServer, 514);
	}

	if (Log::logFile != NULL) {
		fwrite(logentry, strlen(logentry), 1, Log::logFile);
		char newline = '\n';
		fwrite(&newline, 1, 1, Log::logFile);
	}
}

void Log::setLogLevel(LogLevel level) {
	Log::logLevel = level;
}

void Log::d(const char *msg, ...) {
	va_list argptr;
	va_start(argptr, msg);
	Log::log(LEVEL_DEBUG, msg, argptr);
	va_end(argptr);
}

void Log::i(const char *msg, ...) {
	va_list argptr;
	va_start(argptr, msg);
	Log::log(LEVEL_INFO, msg, argptr);
	va_end(argptr);
}

void Log::e(const char *msg, ...) {
	va_list argptr;
	va_start(argptr, msg);
	Log::log(LEVEL_ERROR, msg, argptr);
	va_end(argptr);
}

void Log::enableStdoutDebug(bool enable) {
	Log::stdoutDebug = enable;
}

/**
 * TODO: recuperare da intero
 */
std::string Log::getDateTime() {

	char buf[512];
	time_t now = time(NULL);
	strftime(buf, 512, "%F %T", gmtime(&now));

	return std::string(buf);
}

void Log::updateFromConfig() {
	syslogServer = Config::getSyslogServer();
	syslogEnabled = Config::getSyslogServer() != (uint32_t)0;
}
