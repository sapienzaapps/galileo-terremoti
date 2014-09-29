#ifndef galileo_log_h
#define galileo_log_h

#include "config.h"

FILE *f;
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

#endif
