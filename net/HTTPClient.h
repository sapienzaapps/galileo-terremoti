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

/**
 * HTTP client library
 */
class HTTPClient {
public:

	/**
	 * Send alert to server
	 * @param db Accelerometer values
	 * @param threshold Threshold value
	 */
	static void httpSendAlert(RECORD *db);

	/**
	 * Set server base URL
	 * @param baseUrl New base URL
	 */
	static void setBaseURL(std::string baseUrl);

#ifdef DEBUG

	/**
	 * Send crash reports to server
	 */
	static void sendCrashReports();

#endif

	/**
	 * Returns the base URL used
	 * @return Base URL configured
	 */
	static std::string getBaseURL();

#ifdef DEBUG

	static void *sendCrashReportDoWork(void *mem);

#endif

	static HTTPResponse *httpPostFile(std::string URL, std::string file);

private:
	static unsigned long nextContact;
	static std::string baseUrl;
#ifdef DEBUG
	static pthread_t sendCrashReportThread;
#endif

	static void freeHTTPResponse(HTTPResponse *resp);

	static HTTPResponse *httpRequest(HTTPMethod method, std::string URL, std::map<std::string, std::string> postValues);

	static size_t hostFromURL(const char *url, char *hostname, unsigned short *port);

	static unsigned short getResponseCode(char *line);

	static int getLine(Tcp c, uint8_t *buffer, size_t maxsize, int toRead);

	static int getLine(Tcp c, uint8_t *buffer, size_t maxsize);
};

#endif
