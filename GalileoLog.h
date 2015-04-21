#ifndef galileo_log_h
#define galileo_log_h

#include "config.h"

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

void openLog() {
	f = fopen(log_path, "a");
	if (f == NULL) {
		printf("Error opening file!\n");
		exit(1);
	}
}

void closeLog() {
	fclose(f);
}

void log(char *text) {
	openLog();
	//Serial.println(text);
	fprintf(f, "%s", getGalileoDate());
	fprintf(f, "> %s\n", text);
	closeLog();
}

void logLong(unsigned long text) {
	openLog();
	//Serial.println(text);
	fprintf(f, "%s", getGalileoDate());
	fprintf(f, "> %ld\n", text);

	closeLog();
}

void logInt(int  text) {
	openLog();
	//Serial.println(text);
	fprintf(f, "%s> ", getGalileoDate());
	fprintf(f, "%d\n", text);
	closeLog();
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

#endif
