#ifndef galileo_log_h
#define galileo_log_h

#include <math.h>
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

/**
 * Log class
 */
class Log {
public:
	/**
	 * Log to DEBUG level
	 * @param s String or printf model
	 */
	static void d(const char *s, ...);

	/**
	 * Log to INFO level
	 * @param s String or printf model
	 */
	static void i(const char *s, ...);

	/**
	 * Log to ERROR level
	 * @param s String or printf model
	 */
	static void e(const char *s, ...);

	/**
	 * Get configured SYSLOG
	 * @return SYSLOG ip address
	 */
	static IPaddr getSyslogServer();

	/**
	 * Set log level
	 * @param level New log level
	 */
	static void setLogLevel(LogLevel level);

	/**
	 * Set SYSLOG Server
	 * @param ipaddr New SYSLOG ipaddr
	 */
	static void setSyslogServer(IPaddr ipaddr);

	/**
	 * Switch to a new log file
	 * @param logFilePath New log file path
	 */
	static void setLogFile(std::string logFilePath);

	/**
	 * Switch to a new log file
	 * @param logFilePath New log file path
	 */
	static void setLogFile(const char* logFilePath);

	/**
	 * Enable log to STDOUT
	 * @param b If true, log will be written also to STDOUT
	 */
	static void enableStdoutDebug(bool b);

	/**
	 * Update settings from config
	 */
	static void updateFromConfig();

	/**
	 * Rotate log
	 */
	static void rotate();

	/**
	 * Close log class
	 */
	static void close();

private:
	static void log(LogLevel, const char *, va_list argptr);
	static std::string getDateTime();

	static IPaddr syslogServer;
	static bool syslogEnabled;
	static Udp syslogUdp;
	static std::string logFilePath;
	static FILE *logFile;
	static LogLevel logLevel;
	static bool stdoutDebug;
};

#endif
