#ifndef galileo_log_h
#define galileo_log_h

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <BitsAndBytes.h>
#include <variant.h>
#include "config.h"

#ifndef ARDUINO
#include <Arduino.h>
#endif

FILE *f;
FILE *acc;
static char date_log[30];


char *getGalileoDate() {
  char *cmdDate = "/bin/date +%F%t%T";
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
	static void setSyslogServer(const char*);
	static void setLogFile(const char*);
	static void enableSerialDebug(bool);

private:
	static void log(LogLevel, const char*, va_list argptr);
	static const char* syslogServer;
	static const char* logFilePath;
	static FILE* logFile;
	static bool serialDebug;
	static LogLevel logLevel;
};



const char* Log::syslogServer = NULL;
const char* Log::logFilePath = NULL;
FILE* Log::logFile = NULL;
bool Log::serialDebug = true;
LogLevel Log::logLevel = LEVEL_INFO;

void Log::setSyslogServer(const char *server) {
	Log::syslogServer = server;
}

void Log::setLogFile(const char *filepath) {
	if(Log::logFile != NULL) {
		fclose(Log::logFile);
	}

	Log::logFilePath = filepath;
	Log::logFile = fopen(filepath, "w");
	if (Log::logFile == NULL) {
		Serial.println("Error opening log file\n");
	}
}

void Log::enableSerialDebug(bool serialDebug) {
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
	snprintf(logentry, 1024, "[%s] [%c] %s", getGalileoDate(), level, realmsg);

	if(Log::serialDebug) {
		Serial.println(logentry);
	}

	if(Log::syslogServer != NULL) {
		// TODO: log to syslog
	}

	if(Log::logFile != NULL) {
		// TODO: log to file
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

void log(char* msg) {
	// TODO: remove
	Log::i(msg);
}

void logInt(int l) {
	// TODO: remove
	Log::i("%i", l);
}

void logLong(long l) {
	Log::i("%ld", l);
}

#endif
