#include "httpconn.h"
#include "GalileoLog.h"
#include "commons.h"

#include <fcntl.h>

EthernetClient client;
byte inEvent = 0;
unsigned long nextContact = 5000;

// get data from server to buffer line per line
int getLine(EthernetClient c, char *buffer, int maxsize, int toRead) { //???????
	int i;
	byte done = 0;
	memset(buffer, 0, maxsize);  // set the buffer to 0

	for (i = 0; i < maxsize - 1 && done == 0; i++) {
		buffer[i] = c.read();

		if (buffer[i] == '\r') i--;
		if (buffer[i] == '\n' || buffer[i] == -1) {  // if there is nothing more to read
			done = 1;
			buffer[i] = 0;
		}
		if (toRead == -1) {
			// do nothing: it'll stop only if the buffer is emptied
		}
		else if (toRead > 1) {
			toRead--;
		}
		else {
			done = 1;
		}
	}
	if (toRead == 1) return 1;
	else return i - 1;
}

int getLine(EthernetClient c, char *buffer, int maxsize) {
	return getLine(c, buffer, maxsize, -1);
}

int getPipe(EthernetClient c, char *buffer, int maxsize) {
	return getPipe(c, buffer, maxsize, -1);
}

// get data from server to buffer line per line
int getPipe(EthernetClient c, char *buffer, int maxsize, int toRead) { //???????
	int i;
	byte done = 0;
	memset(buffer, 0, maxsize);  // set the buffer to 0
	for (i = 0; i < maxsize - 1 && done == 0; i++) {
		buffer[i] = c.read();

		if (buffer[i] == '|' /*||  buffer[i] == -1 */) {  // if there is nothing more to read
			done = 1;
			buffer[i] = 0;
		} else if (buffer[i] == -1) return -1; // finished to read at last pipe!
		if (toRead == -1) {
			// do nothing: it'll stop only if the buffer is emptied
		}
		else if (toRead > 1) {
			toRead--;
		}
		else {
			done = 1;
		}
	}
	return i - 1;
}


int prepareFastBuffer(char *buf, struct RECORD *db, struct TDEF *td) {
	// tsstart, deviceid, lat, lon
	Log::d("mac testo: %s", mac_string);
	return sprintf(buf, "tsstart=%u&deviceid=%s&lat=%s&lon=%s", db->ms, mac_string, configGal.lat, configGal.lon);
}

int prepareMacBuffer(char *buf) {
	// tsstart, deviceid, lat, lon
	Log::d("mac testo: %s", mac_string);
	return sprintf(buf, "deviceid=%s", "00000000c1a0");
}

int prepareFirstBuffer(char *buf, struct RECORD *db, struct TDEF *td) {
	// ts, ms, pthresx, pthresy, pthresz, nthresx, nthresy, nthresz, deltax, deltay, deltaz
	return sprintf(buf, "%ld;%ld;%f;%f;%f;%f;%f;%f;%ld;%ld;%ld",
				   db->ts, db->ms, td->pthresx, td->pthresy, td->pthresz, td->nthresx, td->nthresy, td->nthresz,
				   db->valx, db->valy, db->valz);
}

int prepareBuffer(char *buf, struct RECORD *db) {
	// ts, ms, deltax, deltay, deltaz
	return sprintf(buf, "%ld;%ld;%ld;%ld;%ld\r\n", db->ts, db->ms, db->valx, db->valy, db->valz);
}

// open the file into the RAM of the device
int ramopen(int seqid, int sendingIter) {
	// Send values
	char filename[100];
	sprintf(filename, "/media/ram/rel%d_%d.dat", seqid, sendingIter);

	Log::i("Opening file %s", filename);

	return open(filename, O_RDWR | O_CREAT);
}

// remove the file from the RAM of the device
void ramunlink(int seqid, int sendingIter) {
	// Send values
	char filename[100];
	sprintf(filename, "/media/ram/rel%d_%d.dat", seqid, sendingIter);

	Log::i("Removing file %s", filename);

	unlink(filename);
}

// send the accelerometer values that got over the threshold
void httpSendAlert1(struct RECORD *db, struct TDEF *td) {
	// New Event ----------------------------------------------------------
	Log::d("---- httpSendAlert1 ---------START-------");
	Log::i("New Event, values (X-Y-Z):");
	printRecord(db); // Debug print recorded axis values
	Log::d("Date: %s", getGalileoDate());

	char rBuffer[300];
	bool sent = false;
	bool received = false;
	//int connection_status;
	int ntimes = 4; // numbers of connection tryies
// Connecting to server to retrieve "sequence id"
	Log::i("Connecting to:#%s# - num try:%i", httpServer, 4 - ntimes);
	//int connection_status = client.connect(DEFAULT_HTTP_SERVER, 80);
	if (client.connect(DEFAULT_HTTP_SERVER, 80)) { // if connection established
		Log::d("TRYING SENDING NOW!!!!");

		int rsize = prepareFastBuffer(rBuffer, db, td);  // prepare the info for the new entry to be send to DB
		// sendig request to server
		client.print("POST ");
		client.print(DEFAULT_HTTP_PATH);
		client.print("/terremoto.php");
		client.println(" HTTP/1.1");
		client.print("Host: ");
		client.println(DEFAULT_HTTP_SERVER);
		client.println("Content-Type: application/x-www-form-urlencoded");
		client.print("Content-Length: ");
		client.println(rsize);
		client.println("Connection: keep-alive"); // ??? close????
		client.println("");
		client.println(rBuffer);
		Log::d("PATDOMAIN: %s - sending Buffer: %s", DEFAULT_HTTP_PATH, rBuffer);

		sent = true;
		Log::d("Attendiamo i dati... %i", ntimes);

		unsigned long responseMill = millis();

		// Attende che arrivino i dati con timeout nella risposta ***************
		while (!client.available() && (millis() - responseMill < timeoutResponse)) { ; }
		if (millis() - responseMill > timeoutResponse)
			Log::e("TIMEOUT SERVER CONNECTION");
		if (client.available()) { // gestire il caso in cui la connessione con il server avviene ma i dati non arrivano
			// il problema sussiste nel fatto che vengono inviati di nuovo i dati al server
			// client has sent a response
			// Reading headers
			int bodySize = 0;
			int s = getLine(client, rBuffer, 300);
			Log::i("buffer response: %s", rBuffer);

			if (strncmp(rBuffer, "HTTP/1.1 200", 12) == 0) { // risposta ok dal server
				do { // read from client response
					s = getLine(client, rBuffer, 300);
					if (strncmp(rBuffer, "Content-Length", 14) == 0) {
						char *separator = strchr(rBuffer, ':');
						if (*(separator + 1) == ' ') {
							separator = separator + 2;
						} else {
							separator++;
						}
						bodySize = atoi(separator); // get body size response
						//break; // stop
					}
				} while (s > 0); // get data till new line

				s = getLine(client, rBuffer, 300, bodySize); // get content size
				Log::i("rBuffer LENGTH = %i - rBuffer = %ld", s, atol(rBuffer));
				//nextContact = atol(separator+1) + getUNIXTime();  // TIME FOR SENDING COLLECTED DATA
				if (s > 0) {
					nextContact = (atol(rBuffer) * 1000UL);  // get next time to send new data
					received = true;
					Log::i("received = true;");

					inEvent = 1;  // Set ON the Event
					milldelayTimeEvent = millis(); // timestamp in millis for Event Interval */
				}
				Log::d("tempo offset per nextContact: %i - bodySize: %i", nextContact, bodySize);

			} else { // connetion response != 200
				Log::e("connetion response != 200");
				Log::e("Error in reply for req: %s", rBuffer);
				sent = false;
			}
		} else { // client not responding - data not available
			Log::e("Client not available on: sendAlert2");
		}
	} else { // connection to server Failed!!!
		client.stop();
		errors_connection++;
		Log::e("connessione fallita");
	}
	while (client.connected()) {
		Log::d("disconnecting.");
		client.stop();
	}
	Log::d("---- httpSendAlert1------- EXIT ------------");
}

// ask the server for a valid MAC address
void getMacAddressFromServer() {
	Log::i("getMacAddressFromServer() --------------- START------ ");
	if (client.connect(DEFAULT_HTTP_SERVER, 80)) {
		char rBuffer[300];
		int rsize = prepareMacBuffer(rBuffer);

		client.print("POST ");
		client.print(DEFAULT_HTTP_PATH);
		client.print("/alive.php");
		client.println(" HTTP/1.1");
		client.print("Host: ");
		client.println(httpServer);
		client.println("Content-Type: application/x-www-form-urlencoded");
		client.print("Content-Length: ");
		client.println(rsize);
		client.println("Connection: close"); // ???
		client.println("");
		client.println(rBuffer);
		unsigned long responseMill = millis();

		delay(100);
		memset(rBuffer, 0, 300 * sizeof(char));

		while (!client.available() && (millis() - responseMill < timeoutResponse)) { ; }
		if (client.available()) {
			int s = getLine(client, rBuffer, 300);
			Log::i("rBuffer[%i]: %s", s, rBuffer);

			if (strncmp(rBuffer, "HTTP/1.1 200", 12) == 0) {
				int bodySize = 0;
				do {
					s = getLine(client, rBuffer, 300);
					if (strncmp(rBuffer, "Content-Length", 14) == 0) {
						char *separator = strchr(rBuffer, ':');
						if (*(separator + 1) == ' ') {
							separator += 2;
						} else {
							separator++;
						}
						bodySize = atoi(separator);
						Log::i("bodySize: %i", bodySize);
					}
				} while (s > 0);

				// Content
				s = getLine(client, rBuffer, 300, bodySize);
				if (s > 0) {
					Log::i("MAC address: %s", rBuffer);
					memcpy(mac_string, rBuffer, strlen(rBuffer) * sizeof(char)); // copiato mac in stringa
					mac_string[12] = '\0';
				}
				else {
					Log::e("Error getting the content");
				}

			}
			else {
				Log::e("Connection error - not 200!");
			}
		} else {
			Log::e("Client not available on: getMacAddressFromServer");
		}
	} else {
		Log::e("connessione fallita");
	}
	while (client.connected()) {
		Log::d("disconnecting...");
		client.stop();
	}
	Log::i("getMacAddressFromServer() --------------- END ------ ");
}

// given a string made of pair of characters in HEX base, convert them in decimal base - OK
uint8_t *HEXtoDecimal(const char *in, size_t len, uint8_t *out) {
	unsigned int i, t, hn, ln;
	char *ptr;
	char tmp[2];
	long decimal;


	for (t = 0, i = 0; i < len; i += 2, ++t) {
		tmp[0] = in[i];
		tmp[1] = in[i + 1];
		decimal = strtol(tmp, &ptr, 16);

		out[t] = decimal;
		if (decimal < 1) {
			Log::d("0");
		}

		Log::d("%x:", out[t]);
	}

	return out;
}

// read configuration file
void readConfig() {
	Log::i("ON CONFIG READING");

	FILE *fpconf;
	fpconf = fopen(DEFAULT_CONFIG_PATH, "r");
	if (fpconf != NULL) {
		char buffer[100];
		char tmp[30];
		char *argument;
		int latlon = 0;
		while (fgets(buffer, 50, fpconf) != NULL) {
			Log::i("while READING");
			if (strncmp("deviceid", buffer, 8) == 0) {
				argument = strchr(buffer, ':');
				argument++;
				int a = snprintf(tmp, 15, "%s", argument);
				Log::i("tmp[%i]:%s", a, tmp);
				if (a == 13) {
					strncpy(mac_string, tmp, 12); // copio il MAC in formato stringa
					mac_string[12] = '\0';
					HEXtoDecimal(mac_string, strlen(mac_string),
								 mac); // trasformo il mac da stringa ad array di byte HEX

					request_mac_from_server = false; //
					Log::i("Finished mac reading");
				} else {
					Log::e("config request_mac_from_server please!!!!");
					request_mac_from_server = true;
				}
			} else if (!strncmp("lat", buffer, 3)) {
				argument = strchr(buffer, ':');
				argument++;
				strncpy(configGal.lat, argument, (size_t) 8);
				Log::i("latitudine: %lf", configGal.lat);
				latlon++;
			} else if (!strncmp("lon", buffer, 3)) {
				argument = strchr(buffer, ':');
				argument++;
				strncpy(configGal.lon, argument, (size_t) 8);
				Log::i("longitudine: %lf", configGal.lon);
				latlon++;
			}
		}
		if (latlon == 2) {
			request_lat_lon = false;
		}
	} else {
		Log::e("FOPEN FAILED ON CONFIG READING");
	}
}

// convert byte array into C String
void byteMacToString(byte mac_address[]) {
	sprintf(mac_string, "%02x%02x%02x%02x%02x%02x", mac_address[0], mac_address[1], mac_address[2], mac_address[3],
			mac_address[4], mac_address[5]);
	Log::i("MAC ADDRESS TO STRING: %s", mac_string);
}