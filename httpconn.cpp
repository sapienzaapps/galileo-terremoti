#include "httpconn.h"
#include "GalileoLog.h"
#include "commons.h"

#include <fcntl.h>

bool NetworkManager::isDhcpClient = true;
uint8_t* NetworkManager::mac = NULL;
IPAddress NetworkManager::staticAddress(0, 0, 0, 0);
IPAddress NetworkManager::subnetMask(0, 0, 0, 0);
IPAddress NetworkManager::gateway(0, 0, 0, 0);
IPAddress NetworkManager::dnsHost(0, 0, 0, 0);
bool NetworkManager::connectionAvailable = false;
bool NetworkManager::connectionChecked = false;
bool NetworkManager::networkSetup = false;

bool NetworkManager::isConnectedToInternet() {
	return NetworkManager::isConnectedToInternet(false);
}

bool NetworkManager::isConnectedToInternet(bool force) {
	if(!NetworkManager::networkSetup) {
		return false;
	}
	if(!NetworkManager::connectionChecked || force) {
		NetworkManager::connectionAvailable = false;

		int ping = system("bin/busybox ping -w 2 8.8.8.8");

		int pingWifexited = WIFEXITED(ping);
		if (pingWifexited) {
			Log::d("Ping WEXITSTATUS STATUS: %i", WEXITSTATUS(ping));
			if (WEXITSTATUS(ping) == 0) {
				NetworkManager::connectionAvailable = true;
			}
		}
		else {
			Log::d("Ping WEXITSTATUS STATUS: %i", pingWifexited);
		}
		NetworkManager::connectionChecked = true;
	}
	return NetworkManager::connectionAvailable;
}

void NetworkManager::setupAsDHCPClient(uint8_t *mac) {
	boolean isDhcpWorking = false;
	int retry = 0;
	while (!isDhcpWorking && retry < 5) {
		// Trying to get an IP address
		if (Ethernet.begin(mac) == 0) {
			// Error retrieving DHCP IP
			Log::e("Timeout while attempting to discover DHCP server, retrying in 5 seconds...");
			retry++;
			delay(5000);
		} else {
			// DHCP IP retireved successfull
			IPAddress localIp = Ethernet.localIP();
			Log::d("IP retrived successfully from DHCP: %i.%i.%i.%i", localIp[0], localIp[1], localIp[2], localIp[3]);
			isDhcpWorking = true;
		}
	}
	NetworkManager::isDhcpClient = true;
	NetworkManager::networkSetup = isDhcpWorking;
}

void NetworkManager::setupStatic(uint8_t *mac, IPAddress staticAddress, IPAddress subnetMask, IPAddress gateway, IPAddress dnsHost) {
	Log::d("Static configuration: %i.%i.%i.%i/%i.%i.%i.%i gw %i.%i.%i.%i dns %i.%i.%i.%i",
		   staticAddress[0], staticAddress[1], staticAddress[2], staticAddress[3],
		   subnetMask[0], subnetMask[1], subnetMask[2], subnetMask[3],
		   gateway[0], gateway[1], gateway[2], gateway[3],
		   dnsHost[0], dnsHost[1], dnsHost[2], dnsHost[3]
	);

	if(NetworkManager::mac != NULL) {
		free(NetworkManager::mac);
		NetworkManager::mac = NULL;
	}
	NetworkManager::mac = (uint8_t*) malloc(6);
	memcpy(NetworkManager::mac, mac, 6);
	memcpy(&NetworkManager::staticAddress, &staticAddress, sizeof(IPAddress));
	memcpy(&NetworkManager::subnetMask, &subnetMask, sizeof(IPAddress));
	memcpy(&NetworkManager::gateway, &gateway, sizeof(IPAddress));
	memcpy(&NetworkManager::dnsHost, &dnsHost, sizeof(IPAddress));

	NetworkManager::isDhcpClient = false;

	// ARDUINO START CONNECTION
	Ethernet.begin(mac, staticAddress, dnsHost, gateway, subnetMask); // Static address configuration

	char ipAsString[20], maskAsString[20], gwAsString[20], dnsAsString[20];
	snprintf(ipAsString, 20, "%i.%i.%i.%i", staticAddress[0], staticAddress[1], staticAddress[2], staticAddress[3]);
	snprintf(maskAsString, 20, "%i.%i.%i.%i", subnetMask[0], subnetMask[1], subnetMask[2], subnetMask[3]);
	snprintf(gwAsString, 20, "%i.%i.%i.%i", gateway[0], gateway[1], gateway[2], gateway[3]);
	snprintf(dnsAsString, 20, "%i.%i.%i.%i", dnsHost[0], dnsHost[1], dnsHost[2], dnsHost[3]);

	char buf[200];
	memset(buf, 0, 200);
	snprintf(buf, 200, "ifconfig eth0 %s netmask %s up", ipAsString, maskAsString);
	system(buf);

	memset(buf, 0, 200);
	snprintf(buf, 200, "route add default gw %s eth0", gwAsString);
	system(buf);

	memset(buf, 0, 200);
	snprintf(buf, 200, "echo 'nameserver %s' > /etc/resolv.conf", dnsAsString);
	system(buf);

	NetworkManager::networkSetup = true;
}

void NetworkManager::restart() {
#ifdef __IS_GALILEO
	// Workaround for Galileo (and other boards with Linux)
	system("/etc/init.d/networking restart");
	delay(1000);
#endif
	if(NetworkManager::isDhcpClient) {
		NetworkManager::setupAsDHCPClient(NetworkManager::mac);
	} else {
		NetworkManager::setupStatic(
				NetworkManager::mac,
				NetworkManager::staticAddress,
				NetworkManager::subnetMask,
				NetworkManager::gateway,
				NetworkManager::dnsHost
		);
	}
}

void NetworkManager::forceRestart() {
	bool connected;
	int retry = 1;
	do {
		Log::i("Network reset #%i", retry);
		NetworkManager::restart();
		connected = NetworkManager::isConnectedToInternet(true);
		if(retry > 5) {
			system("reboot");
			for(;;);
		} else if(!connected) {
			retry++;
			sleep(2000);
		}
	} while(!connected);
}


unsigned long HTTPClient::nextContact = 5000;

std::string HTTPClient::getConfig() {
	std::string cfg;
	std::map<std::string, std::string> postValues;
	postValues["deviceid"] = Config::getMacAddress();
	postValues["lat"] = Config::getLatitude();
	postValues["lon"] = Config::getLongitude();
	postValues["version"] = SOFTWARE_VERSION;
	postValues["model"] = ARDUINO_MODEL;

	HTTPResponse *resp = httpRequest(HTTP_POST, HTTP_API_ALIVE, postValues);
	if(resp->error == HTTP_OK && resp->body != NULL) {
		cfg = std::string((char*)resp->body);
	} else {
		cfg = std::string("");
	}
	freeHTTPResponse(resp);
	return cfg;
}

// send the accelerometer values that got over the threshold
void HTTPClient::httpSendAlert1(struct RECORD *db, struct TDEF *td) {
	// New Event ----------------------------------------------------------
	Log::d("---- httpSendAlert1 ---------START-------");
	Log::i("New Event, values (X-Y-Z):");
	printRecord(db); // Debug print recorded axis values
	Log::d("Date: %s", getGalileoDate());

	std::map<std::string, std::string> postValues;
	postValues["tsstart"] = db->ms;
	postValues["deviceid"] = Config::getMacAddress();
	postValues["lat"] = Config::getLatitude();
	postValues["lon"] = Config::getLongitude();
	HTTPResponse *resp = httpRequest(HTTP_POST, HTTP_API_TERREMOTO, postValues);
	if(resp->error == HTTP_OK && resp->body != NULL) {
		nextContact = atol((const char*)resp->body) * 1000UL;
	}
	freeHTTPResponse(resp);
}

std::string HTTPClient::getMACAddress() {
	std::string mac;
	std::map<std::string, std::string> postValues;
	postValues["deviceid"] = "00000000c1a0";
	HTTPResponse *resp = httpRequest(HTTP_POST, HTTP_API_ALIVE, postValues);
	if(resp->error == HTTP_OK && resp->body != NULL) {
		mac = std::string((char*)resp->body);
	} else {
		mac = std::string("");
	}
	freeHTTPResponse(resp);
	return mac;
}




unsigned long HTTPClient::getNextContact() {
	return nextContact;
}

size_t HTTPClient::hostFromURL(const char* url, char* hostname, unsigned short* port) {
	size_t offset = 0;
	if(strncmp(url, "http://", 7) == 0) {
		offset += 7;
	}

	size_t urlSize = strlen(url);
	size_t hostEnd = offset;
	while(url[hostEnd] != '/' && url[hostEnd] != ':' && hostEnd < urlSize) {
		hostEnd++;
	}

	memcpy(hostname, url+offset, hostEnd-offset);
	hostname[hostEnd-offset] = 0;

	if(url[hostEnd] == ':') {
		size_t portEnd = hostEnd+1;
		while(url[portEnd] != '/' && portEnd < urlSize) {
			portEnd++;
		}

		char buf[30+1];
		size_t p = portEnd - (hostEnd+1);
		p = (p < 30 ? p : 30);
		memcpy(buf, url+hostEnd+1, p);
		buf[p] = 0;

		*port = (unsigned short)atoi(buf);

		return portEnd;
	} else {
		return hostEnd;
	}
}

unsigned short HTTPClient::getResponseCode(char* line) {
	// HTTP/1.1 200 Ok
	char buf[4];
	memcpy(buf, line + 9, 3);
	buf[4] = 0;
	return (unsigned short)atoi(buf);
}

HTTPResponse* HTTPClient::httpRequest(HTTPMethod method, std::string URL, std::map<std::string, std::string> postValues) {
	HTTPResponse *resp = new HTTPResponse();

	EthernetClient client;
	char serverName[100];
	unsigned short serverPort = 80;
	size_t pathOffset = hostFromURL(URL.c_str(), serverName, &serverPort);

	if(client.connect(serverName, serverPort)) {
		char linebuf[1024];

		snprintf(linebuf, 1024, "%s %s HTTP/1.1", (method == HTTP_GET ? "GET" : "POST"), URL.c_str() + pathOffset);
		client.println(linebuf);

		if(serverPort != 80) {
			snprintf(linebuf, 1024, "Host: %s:%i", serverName, serverPort);
		} else {
			snprintf(linebuf, 1024, "Host: %s", serverName);
		}
		client.println(linebuf);

		client.println("Connection: close");

		if(method == HTTP_POST && postValues.size() == 0) {
			client.println("Content-Length: 0");
			client.println("");
		} else if(method == HTTP_POST && postValues.size() > 0) {
			std::string reqBody;
			for(std::map<std::string, std::string>::iterator i = postValues.begin(); i != postValues.end(); i++) {
				reqBody.append(i->first);
				reqBody.append("=");
				reqBody.append(i->second);
			}
			client.println("Content-Type: application/x-www-form-urlencoded");

			snprintf(linebuf, 1024, "Content-Length: %lu", reqBody.size());
			client.println(linebuf);

			client.println("");
			client.write(reqBody.c_str());
		}

		// Request sent, wait for reply
		unsigned long reqTime = millis();
		while(!client.available() && (millis() - reqTime < HTTP_RESPONSE_TIMEOUT_VALUE ) ){;}

		if(client.available()) {
			char rBuffer[300+1];
			memset(rBuffer, 0, 300+1);
			int s = getLine(client, (uint8_t*)rBuffer, 300);

			Log::i("buffer response[%i]: %s", s, rBuffer);

			if(strncmp(rBuffer, "HTTP/1.1", 8) == 0) {
				resp->error = HTTP_OK;
				resp->responseCode = getResponseCode(rBuffer);

				// Read headers
				int s;
				do {
					s = getLine(client, (uint8_t*)rBuffer, 300);
					if(s > 0 && strlen(rBuffer) != 0) {
						char* dppos = strchr(rBuffer, ':');
						*dppos = 0;
						if(*(dppos+1) == ' ') {
							dppos++;
						}
						dppos++;
						resp->headers[std::string(rBuffer)] = std::string(dppos);
					}
				} while(s > 0 && strlen(rBuffer) != 0);

				resp->body = NULL;
				if(resp->headers.count("Content-Length") == 1) {
					size_t bodySize = (size_t) atol(resp->headers["Content-Length"].c_str());
					resp->body = (uint8_t*)malloc(bodySize);

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
	while(client.connected()) {
		client.stop();
	}
	return resp;
}

void HTTPClient::freeHTTPResponse(HTTPResponse *resp) {
	if(resp->body != NULL) {
		free(resp->body);
	}
	delete resp;
}

// get data from server to buffer line per line
int HTTPClient::getLine(EthernetClient c, uint8_t *buffer, size_t maxsize, int toRead) {
	int i;
	byte done = 0;
	memset(buffer, 0, maxsize);  // set the buffer to 0

	for (i = 0; i < maxsize - 1 && done == 0; i++) {
		buffer[i] = (uint8_t)c.read();

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
			done = 1;
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
