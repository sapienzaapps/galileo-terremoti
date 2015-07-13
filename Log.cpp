#include "Log.h"

IPAddress Log::syslogServer(0, 0, 0, 0);
bool Log::syslogEnabled = false;
EthernetUDP Log::syslogUdp;
FILE *Log::logFile = NULL;
bool Log::serialDebug = true;
LogLevel Log::logLevel = LEVEL_INFO;
std::string Log::deviceid = "";
bool Log::stdoutDebug = false;

void Log::setDeviceId(std::string deviceid) {
	Log::deviceid = deviceid;
}

void Log::setSyslogServer(IPAddress server) {
	Log::syslogServer = server;
	Log::syslogEnabled = true;
	// TODO: choose a special value to disable... maybe 0?
	Log::syslogUdp.begin(514);
}

void Log::setLogFile(const char *filepath) {
	if (Log::logFile != NULL) {
		fclose(Log::logFile);
	}

	Log::logFile = fopen(filepath, "w");
	if (Log::logFile == NULL) {
		Serial.println("Error opening log file\n");
	}
}

void Log::enableSerialDebug(bool serialDebug) {
	if (!Log::serialDebug && serialDebug) {
		Serial.begin(9600);
	} else if (Log::serialDebug && !serialDebug) {
		Serial.end();
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
		snprintf(logentry, 1024, "[%s] [%c] [?] %s", Log::getDateTime(), levelC, realmsg);
	} else {
		snprintf(logentry, 1024, "[%s] [%c] [%s] %s", Log::getDateTime(), levelC, deviceid.c_str(), realmsg);
	}

	if (Log::stdoutDebug) {
		printf("%s\n", logentry);
	}

	if (Log::serialDebug) {
		Serial.println(logentry);
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

		Log::syslogUdp.beginPacket(Log::syslogServer, 514);
		Log::syslogUdp.write((const uint8_t *) pkt, strlen(pkt));
		Log::syslogUdp.endPacket();
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

std::string Log::getDateTime() {
	char *cmdDate = "/bin/date \"+%F %T\"";
	char buf[512];
	memset(buf, 0, 512);
	std::string ret = "";

	FILE *ptr;
	if ((ptr = popen(cmdDate, "r")) != NULL) {
		while (fgets(buf, 64, ptr) != NULL) {
			ret = std::string(buf);
		}
	}

	pclose(ptr);
	return ret;
}
