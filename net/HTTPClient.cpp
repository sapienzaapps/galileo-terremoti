
#include <stdio.h>
#include <string.h>
#include <EthernetClient.h>
#include "HTTPClient.h"
#include "../Log.h"
#include "../common.h"

unsigned long HTTPClient::nextContact = 5000;
std::string HTTPClient::baseUrl = "http://www.sapienzaapps.it/seismocloud/";

std::string HTTPClient::getConfig() {
	std::string cfg;
	std::map<std::string, std::string> postValues;
	postValues["deviceid"] = Config::getMacAddress();
	postValues["lat"] = Config::getLatitude();
	postValues["lon"] = Config::getLongitude();
	postValues["version"] = SOFTWARE_VERSION;
#if GALILEO_GEN == 1
	postValues["model"] = "galileo1";
#else
	postValues["model"] = "galileo2";
#endif

	HTTPResponse *resp = httpRequest(HTTP_POST, baseUrl + "alive.php", postValues);
	if (resp->error == HTTP_OK && resp->body != NULL) {
		cfg = std::string((char *) resp->body);
	} else {
		cfg = std::string("");
	}
	freeHTTPResponse(resp);
	return cfg;
}

// send the accelerometer values that got over the threshold
void HTTPClient::httpSendAlert1(RECORD *db, THRESHOLDS *td) {
	// New Event ----------------------------------------------------------
	Log::d("---- httpSendAlert1 ---------START-------");
	Log::i("New Event, values (X-Y-Z):");
	Log::i("%l - %l - %l", db->valx, db->valy, db->valz);

	std::map<std::string, std::string> postValues;
	postValues["tsstart"] = db->ms;
	postValues["deviceid"] = Config::getMacAddress();
	postValues["lat"] = Config::getLatitude();
	postValues["lon"] = Config::getLongitude();
	HTTPResponse *resp = httpRequest(HTTP_POST, baseUrl + "terremoto.php", postValues);
	if (resp->error == HTTP_OK && resp->body != NULL) {
		nextContact = atol((const char *) resp->body) * 1000UL;
	}
	freeHTTPResponse(resp);
}

std::string HTTPClient::getMACAddress() {
	std::string mac;
	std::map<std::string, std::string> postValues;
	postValues["deviceid"] = "00000000c1a0";
	HTTPResponse *resp = httpRequest(HTTP_POST, baseUrl + "alive.php", postValues);
	if (resp->error == HTTP_OK && resp->body != NULL) {
		mac = std::string((char *) resp->body);
	} else {
		mac = std::string("");
	}
	freeHTTPResponse(resp);
	return mac;
}


unsigned long HTTPClient::getNextContact() {
	return nextContact;
}

size_t HTTPClient::hostFromURL(const char *url, char *hostname, unsigned short *port) {
	size_t offset = 0;
	if (strncmp(url, "http://", 7) == 0) {
		offset += 7;
	}

	size_t urlSize = strlen(url);
	size_t hostEnd = offset;
	while (url[hostEnd] != '/' && url[hostEnd] != ':' && hostEnd < urlSize) {
		hostEnd++;
	}

	memcpy(hostname, url + offset, hostEnd - offset);
	hostname[hostEnd - offset] = 0;

	if (url[hostEnd] == ':') {
		size_t portEnd = hostEnd + 1;
		while (url[portEnd] != '/' && portEnd < urlSize) {
			portEnd++;
		}

		char buf[30 + 1];
		size_t p = portEnd - (hostEnd + 1);
		p = (p < 30 ? p : 30);
		memcpy(buf, url + hostEnd + 1, p);
		buf[p] = 0;

		*port = (unsigned short) atoi(buf);

		return portEnd;
	} else {
		return hostEnd;
	}
}

unsigned short HTTPClient::getResponseCode(char *line) {
	// HTTP/1.1 200 Ok
	char buf[4];
	memcpy(buf, line + 9, 3);
	buf[3] = 0;
	return (unsigned short) atoi(buf);
}

HTTPResponse *HTTPClient::httpRequest(HTTPMethod method, std::string URL,
									  std::map<std::string, std::string> postValues) {
	HTTPResponse *resp = new HTTPResponse();

	EthernetClient client;
	char serverName[100];
	unsigned short serverPort = 80;
	size_t pathOffset = hostFromURL(URL.c_str(), serverName, &serverPort);

	if (client.connect(serverName, serverPort)) {
		char linebuf[1024];

		snprintf(linebuf, 1024, "%s %s HTTP/1.1", (method == HTTP_GET ? "GET" : "POST"), URL.c_str() + pathOffset);
		client.println(linebuf);

		if (serverPort != 80) {
			snprintf(linebuf, 1024, "Host: %s:%i", serverName, serverPort);
		} else {
			snprintf(linebuf, 1024, "Host: %s", serverName);
		}
		client.println(linebuf);

		client.println("Connection: close");

		if (method == HTTP_POST && postValues.size() == 0) {
			client.println("Content-Length: 0");
			client.println("");
		} else if (method == HTTP_POST && postValues.size() > 0) {
			std::string reqBody;
			for (std::map<std::string, std::string>::iterator i = postValues.begin(); i != postValues.end(); i++) {
				reqBody.append(i->first);
				reqBody.append("=");
				reqBody.append(i->second);
			}
			client.println("Content-Type: application/x-www-form-urlencoded");

			snprintf(linebuf, 1024, "Content-Length: %u", reqBody.size());
			client.println(linebuf);

			client.println("");
			client.write(reqBody.c_str());
		}

		// Request sent, wait for reply
		unsigned long reqTime = millis();
		while (!client.available() && (millis() - reqTime < HTTP_RESPONSE_TIMEOUT_VALUE)) { ; }

		if (client.available()) {
			char rBuffer[300 + 1];
			memset(rBuffer, 0, 300 + 1);
			int s = getLine(client, (uint8_t *) rBuffer, 300);

			Log::i("buffer response[%i]: %s", s, rBuffer);

			if (strncmp(rBuffer, "HTTP/1.1", 8) == 0) {
				resp->error = HTTP_OK;
				resp->responseCode = getResponseCode(rBuffer);

				// Read headers
				do {
					s = getLine(client, (uint8_t *) rBuffer, 300);
					if (s > 0 && strlen(rBuffer) != 0) {
						char *dppos = strchr(rBuffer, ':');
						*dppos = 0;
						if (*(dppos + 1) == ' ') {
							dppos++;
						}
						dppos++;
						resp->headers[std::string(rBuffer)] = std::string(dppos);
					}
				} while (s > 0 && strlen(rBuffer) != 0);

				resp->body = NULL;
				if (resp->headers.count("Content-Length") == 1) {
					size_t bodySize = (size_t) atol(resp->headers["Content-Length"].c_str());
					resp->body = (uint8_t *) malloc(bodySize);

					client.read(resp->body, bodySize);
				}
			} else {
				resp->error = HTTP_MALFORMED_REPLY;
			}
		} else {
			resp->error = HTTP_REQUEST_TIMEOUT;
		}
	} else {
		resp->error = HTTP_CONNECTION_TIMEOUT;
	}
	client.stop();
	// TODO: better handling?
	while (client.connected()) {
		client.stop();
	}
	return resp;
}

void HTTPClient::freeHTTPResponse(HTTPResponse *resp) {
	if (resp->body != NULL) {
		free(resp->body);
	}
	delete resp;
}

// get data from server to buffer line per line
int HTTPClient::getLine(EthernetClient c, uint8_t *buffer, size_t maxsize, int toRead) {
	unsigned int i;
	bool done = false;
	memset(buffer, 0, maxsize);  // set the buffer to 0

	for (i = 0; i < maxsize - 1 && !done; i++) {
		buffer[i] = (uint8_t) c.read();

		if (buffer[i] == '\r') {
			i--;
		}

		if (buffer[i] == '\n' || buffer[i] == -1) {  // if there is nothing more to read
			done = 1;
			buffer[i] = 0;
		}

		if (toRead == -1) {
			// do nothing: it'll stop only if the buffer is emptied
		} else if (toRead > 1) {
			toRead--;
		} else {
			done = true;
		}
	}
	if (toRead == 1) {
		return 1;
	} else {
		return i - 1;
	}
}

int HTTPClient::getLine(EthernetClient c, uint8_t *buffer, size_t maxsize) {
	return getLine(c, buffer, maxsize, -1);
}

void HTTPClient::setBaseURL(std::string baseUrl) {
	HTTPClient::baseUrl = baseUrl;
}
