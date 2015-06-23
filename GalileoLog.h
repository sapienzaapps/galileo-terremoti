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
#include "AcceleroMMA7361.h"
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
#include "avg.h"
#include "GalileoLog.h"

FILE *f;
FILE *acc;
static char date_log[30];


char *getGalileoDate() {
  char *cmdDate = "/bin/date \"+%F %T\"";
  char buf[64];
  //char *date = (char*)malloc(22*sizeof(char)); memory leak ****
  memset(date_log, 0, 22); // zero memory buffer
  strcpy(date_log, "");
  FILE *ptr;

  if ((ptr = popen(cmdDate, "r")) != NULL)
  {
    while (fgets(buf, 64, ptr) != NULL)
    {
      strcat(date_log, buf);
      //Serial.print(buf);
    }
  }

  (void) pclose(ptr);

  //strcat(date_log, " > ");
  //strcat(date_log, (char *)'\0');
  date_log[strlen(date_log)-1] = 0;
  return date_log;
}

void logAccValues(long _valx, long _valy, long _valz, byte zz){ // function to log 1h of acceleration force
  // FILE *acc;
  // acc = fopen(logAcc_path, "a");
  // if (acc == NULL) {
    // printf("Error opening file!\n");
    // exit(1);
  // }
  if(zz == 0){ // FIRST
    
    acc = fopen(logAcc_path, "a");
    if (acc == NULL) {
      printf("Error opening file!\n");
      exit(1);
    }
    fprintf(acc, "%s > %s\n ", getGalileoDate(), "#####Starting logging for 1h#####" );
  }else if(zz == 1){// NORMAL
    fprintf(acc, "%lu , %lu , %lu\n ", _valx, _valy, _valz);
  }else if(zz == 2){ // LAST
    fprintf(acc, "%s> %s\n ", getGalileoDate(), "#####Finished logging after 1h#####" );
    fclose(acc);
  }
  // fclose(acc);
}

typedef enum _LogLevel {
	LEVEL_DEBUG = 0,
	LEVEL_INFO = 1,
	LEVEL_ERROR = 2
} LogLevel;

class Log {
public:
	static void d(const char*, ...);
	static void i(const char*, ...);
	static void e(const char*, ...);

	static void setLogLevel(LogLevel);
	static void setSyslogServer(IPAddress);
	static void setLogFile(const char*);
	static void enableSerialDebug(bool);
	static void setDeviceId(char*);

private:
	static void log(LogLevel, const char*, va_list argptr);
	static IPAddress syslogServer;
	static bool syslogEnabled;
	static EthernetUDP syslogUdp;
	static const char* logFilePath;
	static FILE* logFile;
	static bool serialDebug;
	static LogLevel logLevel;
	static char* deviceid;
};



IPAddress Log::syslogServer(0, 0, 0, 0);
bool Log::syslogEnabled = false;
EthernetUDP Log::syslogUdp;
FILE* Log::logFile = NULL;
bool Log::serialDebug = true;
LogLevel Log::logLevel = LEVEL_INFO;
char* Log::deviceid = NULL;

void Log::setDeviceId(char *deviceid) {
	Log::deviceid = deviceid;
}

void Log::setSyslogServer(IPAddress server) {
	Log::syslogServer = server;
	Log::syslogEnabled = true;
	// TODO: choose a special value to disable... maybe 0?
	Log::syslogUdp.begin(514);
}

void Log::setLogFile(const char *filepath) {
	if(Log::logFile != NULL) {
		fclose(Log::logFile);
	}

	Log::logFile = fopen(filepath, "w");
	if (Log::logFile == NULL) {
		Serial.println("Error opening log file\n");
	}
}

void Log::enableSerialDebug(bool serialDebug) {
	if(!Log::serialDebug && serialDebug) {
		Serial.begin(9600);
	} else if(Log::serialDebug && !serialDebug) {
		Serial.end();
	}
	Log::serialDebug = serialDebug;
}

void Log::log(LogLevel level, const char *msg, va_list argptr) {
	// Skip messages with low log priority
	if(Log::logLevel > level) {
		return;
	}

	// Preparing log message with arguments
	char realmsg[1024];
	vsnprintf(realmsg, 1024, msg, argptr);

	//Prepending log message with datetime and level
	char logentry[1024];
	char levelC = 'D';
	if(level == LEVEL_INFO) {
		levelC = 'I';
	} else if(level == LEVEL_ERROR) {
		levelC = 'E';
	}
	if(deviceid == NULL) {
		snprintf(logentry, 1024, "[%s] [%c] [?] %s", getGalileoDate(), levelC, realmsg);
	} else {
		snprintf(logentry, 1024, "[%s] [%c] [%s] %s", getGalileoDate(), levelC, deviceid, realmsg);
	}

	if(Log::serialDebug) {
		Serial.println(logentry);
	}

	if(Log::syslogEnabled) {
		// priority = (facility * 8) + severity
		int priority = (16 * 8);
		if(level == LEVEL_INFO) {
			priority += 6;
		} else if(level == LEVEL_ERROR) {
			priority += 2;
		} else {
			priority += 7;
		}

		// 1024 is the maximum size of syslog UDP entry
		char pkt[1024];
		snprintf(pkt, 1024, "<%i>%s", priority, logentry);

		Log::syslogUdp.beginPacket(Log::syslogServer, 514);
		Log::syslogUdp.write((const uint8_t*)pkt, strlen(pkt));
		Log::syslogUdp.endPacket();
	}

	if(Log::logFile != NULL) {
		fwrite(logentry, strlen(logentry), 1, Log::logFile);
		char newline = '\n';
		fwrite(&newline, 1, 1, Log::logFile);
	}
}

void Log::setLogLevel(LogLevel level) {
	Log::logLevel = level;
}

void Log::d(const char* msg, ...) {
	va_list argptr;
	va_start(argptr, msg);
	Log::log(LEVEL_DEBUG, msg, argptr);
	va_end(argptr);
}

void Log::i(const char* msg, ...) {
	va_list argptr;
	va_start(argptr, msg);
	Log::log(LEVEL_INFO, msg, argptr);
	va_end(argptr);
}

void Log::e(const char* msg, ...) {
	va_list argptr;
	va_start(argptr, msg);
	Log::log(LEVEL_ERROR, msg, argptr);
	va_end(argptr);
}

#endif
