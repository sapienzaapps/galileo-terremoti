#ifndef httpconn_h
#define httpconn_h 1

#include <string>
#include <map>
#include "Tcp.h"
#include "../Seismometer.h"

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

typedef struct {
	HTTPError error;
	unsigned short responseCode;
	std::map<std::string, std::string> headers;
	uint8_t *body;
} HTTPResponse;

class HTTPClient {
public:
	static std::string getConfig();
	static void httpSendAlert1(RECORD *db, THRESHOLDS *td);
	static void setBaseURL(std::string baseUrl);
	static void sendCrashReports();
	static std::string getBaseURL();
private:
	static unsigned long nextContact;
	static std::string baseUrl;

	static void freeHTTPResponse(HTTPResponse *resp);
	static HTTPResponse *httpRequest(HTTPMethod method, std::string URL, std::map<std::string, std::string> postValues);
	static HTTPResponse *httpRequestWithFile(std::string file);
	static size_t hostFromURL(const char *url, char *hostname, unsigned short *port);
	static unsigned short getResponseCode(char *line);
	static int getLine(Tcp c, uint8_t *buffer, size_t maxsize, int toRead);
	static int getLine(Tcp c, uint8_t *buffer, size_t maxsize);
};

#endif
