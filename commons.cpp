#include "commons.h"
#include "httpconn.h"

IPAddress ip;
IPAddress dnsServer;
IPAddress gateway;
IPAddress subnet;

char *httpServer;
IPAddress timeServer;

FILE *script;

unsigned long resetConnetcionMills = 0;
unsigned long resetConnectionInterval = 3 * 1000;
unsigned long milldelayTimeEvent = 0;

//printing a record state
void printRecord(struct RECORD *db) {
	Log::d("%ld:%ld:%ld", db->valx, db->valy, db->valz);
}

// check if a file exists
int doesFileExist(const char *filename) {
	if (access(filename, F_OK) != -1) {
		// file exists
		return 1;
	}
	else {
		// file doesn't exist
		return 0;
	}
}

// create a script
void createScript(const char *path, const char *text) {
	if (path == NULL) {// DOWNLOADED CREATION SCRIPT
		script = fopen(script_path, "w");
	} else {
		script = fopen(path, "w");
	}
	if (script == NULL) {
		Log::d("F_Error opening script!\n");
		//exit(1);
		return;
	} else {
		Log::d(text);
		//fprintf(script, "%s> ", getGalileoDate());
		fprintf(script, "%s\n", text);
		fclose(script);

		delay(5);

		char *rights = "chmod a+rx %s";
		char str[80];
		int len = sprintf(str, rights, path);

		Log::d("createScript - bytes written: %d", len);

		//str[len] = '\0';// togliere!!!??
		//system("chmod a+rx /gscript/prova.sh");

		system(str);
		Log::i("chmod a+rx script file");
	}
}

// execute a script
void execScript(const char *path) {
	if (doesFileExist(path)) {
		Log::d("Executing script: %s", path);
		delay(5);
		system(path);
		delay(500);
	} else {
		Log::e("script not found!!!");
	}
}

// RESET CONNECTION IF IS NOT CONNECTED TO INTERNET
void resetConnection(int numTry) {
	if (millis() - resetConnetcionMills > resetConnectionInterval) {
		resetConnetcionMills = millis();
		Log::i("Trying to restore INTERNET CONNECTION: %i", numTry);
		if (numTry % 2 == 0) {  // LED RESET CONNECTION BLINK
			digitalWrite(10, LOW);
			digitalWrite(12, HIGH);

		} else {
			digitalWrite(10, HIGH);
			digitalWrite(12, LOW);
		}
		// Workaround for Galileo (and other boards with Linux)
		system("/etc/init.d/networking restart");
		delay(3000);
		bool internetConnected = NetworkManager::isConnectedToInternet();
		if (!internetConnected) {// TEST TEST TEST TEST!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
			Log::e("---- FAILED ----  restore Internet connection");
			Log::e("+++++ setupEthernet() ++++++");

			//setupEthernet();// After network restart try to set Arduino network
			delay(1000);
			if (!(NetworkManager::isConnectedToInternet()) && numTry <= 0) {
				// TRYING TO REBOOT DEVICE
				Log::e("----- REBOOT GALILEO -----");

				//system("reboot");
				if (!doesFileExist(script_reset)) { // check if reboot script exists
					createScript(script_path, reboot_scriptText);
					delay(5);
				}
				//system("/gscript/reset.sh");
				system(script_reset);
				while (1) { ; } // lock HERE for  SYSTEM RESET
			}
		} else {
			Log::i("---- SUCCESS ----  restore Internet connection");
		}
	}
}


//////////////////////////////////////////////////////////////////////////////
// Yet, another good itoa implementation
// returns: the length of the number string
int itoa(int value, char *sp, int radix) {
	char tmp[16];// be careful with the length of the buffer
	char *tp = tmp;
	int i;
	unsigned v;

	int sign = (radix == 10 && value < 0);
	if (sign)
		v = -value;
	else
		v = (unsigned) value;

	while (v || tp == tmp) {
		i = v % radix;
		v /= radix; // v/=radix uses less CPU clocks than v=v/radix does
		if (i < 10)
			*tp++ = i + '0';
		else
			*tp++ = i + 'a' - 10;
	}

	int len = tp - tmp;

	if (sign) {
		*sp++ = '-';
		len++;
	}

	while (tp > tmp)
		*sp++ = *--tp;

	return len;
}

///////////////////////////////////////////////////////////////////////////////

/* ################################################################################################## */

char *floatToString(char *outstr, float value, int places) {
	return floatToString(outstr, value, places, 0);
}

char *floatToString(char *outstr, float value, int places, int minwidth) {
	return floatToString(outstr, value, places, minwidth, false);
}

char *floatToString(char *outstr, float value, int places, int minwidth, boolean rightjustify) {
	// this is used to write a float value to string, outstr.  oustr is also the return value.
	int digit;
	float tens = 0.1;
	int tenscount = 0;
	int i;
	float tempfloat = value;
	int c = 0;

	int charcount = 1;
	int extra = 0;
	char countdown[10];
	// make sure we round properly. this could use pow from <math.h>, but doesn't seem worth the import
	// if this rounding step isn't here, the value  54.321 prints as 54.3209

	// calculate rounding term d:   0.5/pow(10,places)
	float d = 0.5;
	if (value < 0) d *= -1.0;
	// divide by ten for each decimal place
	for (i = 0; i < places; i++)
		d /= 10.0;
	// this small addition, combined with truncation will round our values properly
	tempfloat += d;

	// first get value tens to be the large power of ten less than value
	if (value < 0) tempfloat *= -1.0;
	while ((tens * 10.0) <= tempfloat) {
		tens *= 10.0;
		tenscount += 1;
	}

	if (tenscount > 0) charcount += tenscount;
	else charcount += 1;

	if (value < 0)
		charcount += 1;
	charcount += 1 + places;

	minwidth += 1; // both count the null final character
	if (minwidth > charcount) {
		extra = minwidth - charcount;
		charcount = minwidth;
	}

	if (extra > 0 && rightjustify) {
		for (int i = 0; i < extra; i++) {
			outstr[c++] = ' ';
		}
	}

	// write out the negative if needed
	if (value < 0) outstr[c++] = '-';

	if (tenscount == 0) outstr[c++] = '0';

	for (i = 0; i < tenscount; i++) {
		digit = (int) (tempfloat / tens);
		itoa(digit, &outstr[c++], 10);
		tempfloat = tempfloat - ((float) digit * tens);
		tens /= 10.0;
	}
	// if no places after decimal, stop now and return

	// otherwise, write the point and continue on
	if (places > 0)outstr[c++] = '.';
	// now write out each decimal place by shifting digits one by one into the ones place and writing the truncated value
	for (i = 0; i < places; i++) {
		tempfloat *= 10.0;
		digit = (int) tempfloat;
		itoa(digit, &outstr[c++], 10);
		// once written, subtract off that digit
		tempfloat = tempfloat - (float) digit;
	}
	if (extra > 0 && !rightjustify) {
		for (int i = 0; i < extra; i++) {
			outstr[c++] = ' ';
		}
	}
	outstr[c++] = '\0';
	return outstr;
}

// store the given MAC address to a FILE into the SD card
void storeConfigToSD() {
	FILE *fp = fopen(DEFAULT_CONFIG_PATH, "w");
	if (fp == NULL) {
		printf("Error opening file!\n");
		exit(1);
	}
	Log::i("Store lat lon mac: %s %s %s", Config::getLatitude(), Config::getLongitude(), Config::getMacAddress().c_str());
	fprintf(fp, "deviceid:%s\nlat:%lf\nlon:%lf", Config::getMacAddress().c_str(), Config::getLatitude(), Config::getLongitude());
	fclose(fp);
}

void ipToString(char *buf, size_t maxsize, IPAddress addr) {
	snprintf(buf, maxsize, "%i.%i.%i.%i", addr[0], addr[1], addr[2], addr[3]);
}

void printConfig() {
	Log::i("###################### Config ######################### ");
	Log::i("UDID (DeviceID): %s - Model: %s - Version: %s", Config::getMacAddress().c_str(), ARDUINO_MODEL, SOFTWARE_VERSION);
	Log::i("Position (lat, lon): %lf %lf", Config::getLatitude(), Config::getLongitude());

	char buf[300];
	IPAddress localIp = Ethernet.localIP();
	snprintf(buf, 300, "%i.%i.%i.%i", localIp[0], localIp[1], localIp[2], localIp[3]);

	Log::i("IP: %s - Connection errors: %i", buf, errors_connection);
	showThresholdValues();
	Log::i("##################### Config end ####################### ");
}

void resetBlink(byte type) {
	if (type) { // if 1 reset
		digitalWrite(LED_RED, HIGH);
		digitalWrite(LED_GREEN, LOW);
		delay(500);
		digitalWrite(LED_RED, LOW);
		digitalWrite(LED_GREEN, HIGH);
		delay(500);
		digitalWrite(LED_RED, HIGH);
		digitalWrite(LED_GREEN, LOW);
		delay(500);
		digitalWrite(LED_RED, LOW);
		digitalWrite(LED_GREEN, HIGH);
		delay(1500);
	} else { // if 0 update
		digitalWrite(LED_RED, HIGH);
		digitalWrite(LED_GREEN, HIGH);
		delay(500);
		digitalWrite(LED_RED, LOW);
		digitalWrite(LED_GREEN, LOW);
		delay(500);
		digitalWrite(LED_RED, HIGH);
		digitalWrite(LED_GREEN, HIGH);
		delay(500);
		digitalWrite(LED_RED, LOW);
		digitalWrite(LED_GREEN, LOW);
		delay(1000);
	}
}

double atofn(char* buf, size_t max) {
	size_t bufLen = strlen(buf);
	max = (max <= bufLen ? max : bufLen);
	char termination = buf[max];
	buf[max] = 0;
	double ret = atof(buf);
	buf[max] = termination;
	return ret;
}
