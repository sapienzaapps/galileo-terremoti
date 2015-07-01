#ifndef galileo_log_h
#define galileo_log_h

#include "buildcfg.h"

#include <math.h>
#include <sys/sysinfo.h>
#include <signal.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>

#ifdef __IS_GALILEO

#include <Arduino.h>

#include <pins_arduino.h>
#include <BitsAndBytes.h>
#include <Ethernet.h>
#include <SPI.h>
#include <EEPROM.h>
#include <SD.h>
#include <EthernetUdp.h>

#endif

#include "config.h"

char *getGalileoDate();
void logAccValues(long _valx, long _valy, long _valz, byte zz);

typedef enum _LogLevel {
	LEVEL_DEBUG = 0,
	LEVEL_INFO = 1,
	LEVEL_ERROR = 2
} LogLevel;

class Log {
public:
	static void d(const char *, ...);
	static void i(const char *, ...);
	static void e(const char *, ...);

	static void setLogLevel(LogLevel);
	static void setSyslogServer(IPAddress);
	static void setLogFile(const char *);
	static void enableSerialDebug(bool);
	static void enableStdoutDebug(bool);
	static void setDeviceId(char *);

private:
	static void log(LogLevel, const char *, va_list argptr);
	static IPAddress syslogServer;
	static bool syslogEnabled;
	static EthernetUDP syslogUdp;
	static const char *logFilePath;
	static FILE *logFile;
	static bool serialDebug;
	static LogLevel logLevel;
	static char *deviceid;
	static bool stdoutDebug;
};

#endif
