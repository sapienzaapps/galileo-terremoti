#ifndef COMMONS_H_
#define COMMONS_H_

#include "threshold.h"

extern byte errors_connection;

// struct for time and axis variations logging
struct RECORD {
	unsigned long ts;
	unsigned long ms;
	long valx;
	long valy;
	long valz;
	boolean overThreshold;
};

struct TDEF {
	double pthresx;
	double pthresy;
	double pthresz;
	double nthresx;
	double nthresy;
	double nthresz;
};

extern IPAddress ip;
extern IPAddress dnsServer;
extern IPAddress gateway;
extern IPAddress subnet;

extern char *httpServer;
extern IPAddress timeServer;

extern FILE *script;

// TODO: uppercase
#define script_path "media/realroot/prova.sh"
#define script_reset "media/realroot/reset.sh"
#define reboot_scriptText "#!/bin/bash\nshutdown -r -t sec 00\n"
#define download_scriptText "curl -o /media/realroot/sketch.elf  %s"

extern unsigned long resetConnetcionMills;
extern unsigned long resetConnectionInterval;
extern unsigned long milldelayTimeEvent;

void printRecord(struct RECORD *db);
int doesFileExist(const char *filename);
void createScript(const char *path, const char *text);
void execScript(const char *path);
int itoa(int value, char *sp, int radix);
char *floatToString(char *outstr, float value, int places, int minwidth, boolean rightjustify);
char *floatToString(char *outstr, float value, int places, int minwidth);
void storeConfigToSD();
void printConfig();
void resetBlink(byte type);
double atofn(char *buf, size_t max);

#endif 
