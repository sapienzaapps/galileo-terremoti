
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include "HTTPClient.h"
#include "../Log.h"
#include "../Utils.h"

unsigned long HTTPClient::nextContact = 5000;
#ifdef DEBUG
std::string HTTPClient::baseUrl = "http://192.0.2.20/seismocloud/";
#else
std::string HTTPClient::baseUrl = "http://www.sapienzaapps.it/seismocloud/";
#endif

std::string HTTPClient::getConfig() {
	std::string cfg;
	std::map<std::string, std::string> postValues;
	postValues["deviceid"] = Config::getMacAddress();
	postValues["lat"] = Utils::toString(Config::getLatitude());
	postValues["lon"] = Utils::toString(Config::getLongitude());
	postValues["version"] = SOFTWARE_VERSION;
	postValues["memfree"] = Utils::toString(Utils::getFreeRam());
	postValues["uptime"] = Utils::toString(Utils::uptime());
	postValues["model"] = PLATFORM_TAG;
	postValues["sensor"] = Seismometer::getInstance()->getAccelerometerName();

	HTTPResponse *resp = httpRequest(HTTP_POST, baseUrl + "alive.php", postValues);
	Log::d("Response received, code: %i", resp->responseCode);
	if (resp->error == HTTP_OK && resp->responseCode == 200 && resp->body != NULL) {
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

HTTPResponse *HTTPClient::httpRequest(HTTPMethod method, std::string URL, std::map<std::string, std::string> postValues) {
	HTTPResponse *resp = new HTTPResponse();

	Tcp client;
	char serverName[100];
	unsigned short serverPort = 80;
	size_t pathOffset = hostFromURL(URL.c_str(), serverName, &serverPort);

	if (client.connectTo(std::string(serverName), serverPort)) {
		Log::d("Connect to server OK");
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
				reqBody.append("&");
			}
			reqBody = Utils::trim(reqBody, '&');
			client.println("Content-Type: application/x-www-form-urlencoded");

			unsigned long contentLength = reqBody.size();
			snprintf(linebuf, 1024, "Content-Length: %lu", contentLength);
			client.println(linebuf);

			client.println("");
			client.print(reqBody.c_str());
		}

		Log::d("HTTP Request to %s:%i sent", serverName, serverPort);

		// Request sent, wait for reply
		unsigned long reqTime = Utils::millis();
		while (!client.available() && (Utils::millis() - reqTime < HTTP_RESPONSE_TIMEOUT_VALUE)) { ; }

		if (client.available()) {
			char rBuffer[300 + 1];
			memset(rBuffer, 0, 300 + 1);
			int s = getLine(client, (uint8_t *) rBuffer, 300);

			Log::d("buffer response[%i]: %s", s, rBuffer);

			if (strncmp(rBuffer, "HTTP/1.", 7) == 0) {
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
					resp->body = (uint8_t *) malloc(bodySize+1);
					memset(resp->body, 0, bodySize+1);
					client.readall(resp->body, bodySize);
				}
			} else {
				Log::e("HTTP malformed reply");
				resp->error = HTTP_MALFORMED_REPLY;
			}
		} else {
			Log::e("HTTP request timeout");
			resp->error = HTTP_REQUEST_TIMEOUT;
		}
	} else {
		Log::e("HTTP connection timeout");
		resp->error = HTTP_CONNECTION_TIMEOUT;
	}
	Log::d("Stopping tcp client");
	client.stop();
	// TODO: better handling?
	while (client.connected()) {
		client.stop();
	}
	return resp;
}

HTTPResponse *HTTPClient::httpPostFile(std::string URL, std::string file) {
	HTTPResponse *resp = new HTTPResponse();

	Tcp client;
	char serverName[100];
	unsigned short serverPort = 80;
	size_t pathOffset = hostFromURL(URL.c_str(), serverName, &serverPort);

	if (client.connectTo(std::string(serverName), serverPort)) {
		Log::d("Connect to server OK");
		char linebuf[1024];

		snprintf(linebuf, 1024, "POST %s HTTP/1.1", URL.c_str() + pathOffset);
		client.println(linebuf);

		if (serverPort != 80) {
			snprintf(linebuf, 1024, "Host: %s:%i", serverName, serverPort);
		} else {
			snprintf(linebuf, 1024, "Host: %s", serverName);
		}
		client.println(linebuf);

		client.println("Content-Type: text/plain");
		client.println("Connection: close");

		off_t fileSize = Utils::fileSize(file.c_str());
		std::string contentLength = "Content-Length: " + Utils::toString((int)fileSize);
		client.println(contentLength.c_str());
		client.println("");

		FILE *fp = fopen(file.c_str(), "r");
		char buf[fileSize];
		memset(buf, 0, fileSize);
		fread(buf, fileSize, 1, fp);
		client.send(buf, fileSize);
		fclose(fp);

		Log::d("HTTP Request to %s sent", URL.c_str());

		// Request sent, wait for reply
		unsigned long reqTime = Utils::millis();
		while (!client.available() && (Utils::millis() - reqTime < HTTP_RESPONSE_TIMEOUT_VALUE)) { ; }

		if (client.available()) {
			char rBuffer[300 + 1];
			memset(rBuffer, 0, 300 + 1);
			int s = getLine(client, (uint8_t *) rBuffer, 300);

			Log::d("buffer response[%i]: %s", s, rBuffer);

			if (strncmp(rBuffer, "HTTP/1.", 7) == 0) {
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
					resp->body = (uint8_t *) malloc(bodySize+1);
					memset(resp->body, 0, bodySize+1);
					client.readall(resp->body, bodySize);
				}
			} else {
				Log::e("HTTP malformed reply");
				resp->error = HTTP_MALFORMED_REPLY;
			}
		} else {
			Log::e("HTTP request timeout");
			resp->error = HTTP_REQUEST_TIMEOUT;
		}
	} else {
		Log::e("HTTP connection timeout");
		resp->error = HTTP_CONNECTION_TIMEOUT;
	}
	Log::d("Stopping tcp client");
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
int HTTPClient::getLine(Tcp c, uint8_t *buffer, size_t maxsize, int toRead) {
	unsigned int i;
	bool done = false;
	memset(buffer, 0, maxsize);  // set the buffer to 0

	for (i = 0; i < maxsize - 1 && !done; i++) {
		int bufchar = c.readchar();
		buffer[i] = (uint8_t) bufchar;

		if (buffer[i] == '\r') {
			i--;
		} else if (buffer[i] == '\n' || bufchar == -1) {  // if there is nothing more to read
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

int HTTPClient::getLine(Tcp c, uint8_t *buffer, size_t maxsize) {
	return getLine(c, buffer, maxsize, -1);
}

void HTTPClient::setBaseURL(std::string baseUrl) {
	size_t len = baseUrl.size();
	if(baseUrl[len-1] != '/') {
		baseUrl.append("/");
	}
	HTTPClient::baseUrl = baseUrl;
}

std::string HTTPClient::getBaseURL() {
	return baseUrl;
}

void HTTPClient::sendCrashReports() {
	// TODO
	struct dirent *entry;
	DIR *dp = opendir(WATCHDOG_CRASHDIR);
	if(dp == NULL) {
		return;
	}

	while((entry = readdir(dp))) {
		std::string filename = std::string(WATCHDOG_CRASHDIR) + "/" + entry->d_name;

		HTTPResponse *resp = httpPostFile(baseUrl + "crashreport.php?deviceid=" + Utils::getInterfaceMAC(), filename);
		if(resp->error == HTTP_OK && resp->responseCode == 200) {
			unlink(filename.c_str());
		}
		freeHTTPResponse(resp);
	}

	closedir(dp);
}
