#ifndef galileo_log_h
#define galileo_log_h

#include <math.h>
#include <sys/sysinfo.h>
#include <signal.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <string>
#include "net/IPaddr.h"
#include "net/Udp.h"

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

	static IPaddr getSyslogServer();
	static void setLogLevel(LogLevel);
	static void setSyslogServer(IPaddr);
	static void setLogFile(std::string);
	static void setLogFile(const char*);
	static void enableStdoutDebug(bool);
	static void setDeviceId(std::string);
	static void updateFromConfig();
	static void rotate();

private:
	static void log(LogLevel, const char *, va_list argptr);
	static std::string getDateTime();

	static IPaddr syslogServer;
	static bool syslogEnabled;
	static Udp syslogUdp;
	static std::string logFilePath;
	static FILE *logFile;
	static LogLevel logLevel;
	static std::string deviceid;
	static bool stdoutDebug;
};

#endif
