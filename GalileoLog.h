#ifndef galileo_log_h
#define galileo_log_h

FILE *f;

char *getGalileoDate() {
	char *cmd2 = "/bin/date +%F%t%T";
	char buf[64];
	char *date = (char*)malloc(22*sizeof(char));
	strcpy(date, "");
	FILE *ptr;

	if ((ptr = popen(cmd2, "r")) != NULL)
	{
		while (fgets(buf, 64, ptr) != NULL)
		{
			strcat(date, buf);
			//Serial.print(buf);
		}
	}

	(void) pclose(ptr);

	date[strlen(date)-1] = 0;
	strcat(date, " > ");
	return date;
}

void openLog() {
	f = fopen("log.txt", "a");
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
	fprintf(f, "%s\n", text);
	closeLog();
}

void logLong(unsigned long text) {
	openLog();
	//Serial.println(text);
	fprintf(f, "%s", getGalileoDate());
	fprintf(f, "%ld\n", text);
	closeLog();
}

#endif
