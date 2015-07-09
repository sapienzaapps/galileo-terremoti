#ifndef httpconn_h
#define httpconn_h 1

#include <string>
#include <map>
#include <Arduino.h>
#include <Ethernet.h>

class NetworkManager {
public:
	static bool isConnectedToInternet();
	static bool isConnectedToInternet(bool force);
	static void setupAsDHCPClient(uint8_t *mac);
	static void setupStatic(uint8_t *mac, IPAddress staticAddress, IPAddress subnetMask, IPAddress gateway,
							IPAddress dnsHost);
	static void restart();
	static void forceRestart();
private:
	static bool networkSetup;
	static bool connectionAvailable;
	static bool connectionChecked;
	static bool isDhcpClient;
	static uint8_t *mac;
	static IPAddress staticAddress;
	static IPAddress subnetMask;
	static IPAddress gateway;
	static IPAddress dnsHost;
};

typedef enum {
	HTTP_OK,
	HTTP_CONNECTION_TIMEOUT,
	HTTP_REQUEST_TIMEOUT,
	HTTP_MALFORMED_REPLY
} HTTPError;

typedef enum {
	HTTP_GET,
	HTTP_POST
} HTTPMethod;

typedef struct HTTPResponse {
	HTTPError error;
	unsigned short responseCode;
	std::map<std::string, std::string> headers;
	uint8_t *body;
};

class HTTPClient {
public:
	static std::string getConfig();
	static std::string getMACAddress();
	static void httpSendAlert1(struct RECORD *db, struct TDEF *td);
	static unsigned long getNextContact();
	static void setBaseURL(std::string baseUrl);
private:
	static unsigned long nextContact;
	static std::string baseUrl;

	static void freeHTTPResponse(HTTPResponse *resp);
	static HTTPResponse *httpRequest(HTTPMethod method, std::string URL, std::map<std::string, std::string> postValues);
	static size_t hostFromURL(const char *url, char *hostname, unsigned short *port);
	static unsigned short getResponseCode(char *line);
	static int getLine(EthernetClient c, uint8_t *buffer, size_t maxsize, int toRead);
	static int getLine(EthernetClient c, uint8_t *buffer, size_t maxsize);
};

#endif
