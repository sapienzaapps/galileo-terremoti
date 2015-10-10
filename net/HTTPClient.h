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
	 * Get config from server
	 * @return The config as string (as returned to server)
	 */
	static std::string getConfig();

	/**
	 * Send alert to server
	 * @param db Accelerometer values
	 * @param td Threshold values
	 */
	static void httpSendAlert1(RECORD *db, THRESHOLDS *td);

	/**
	 * Set server base URL
	 * @param baseUrl New base URL
	 */
	static void setBaseURL(std::string baseUrl);

	/**
	 * Send crash reports to server
	 */
	static void sendCrashReports();

	/**
	 * Returns the base URL used
	 * @return Base URL configured
	 */
	static std::string getBaseURL();

	static void *sendCrashReportDoWork(void* mem);
private:
	static unsigned long nextContact;
	static std::string baseUrl;
	static pthread_t sendCrashReportThread;

	static void freeHTTPResponse(HTTPResponse *resp);
	static HTTPResponse *httpRequest(HTTPMethod method, std::string URL, std::map<std::string, std::string> postValues);
	static HTTPResponse *httpPostFile(std::string URL, std::string file);
	static size_t hostFromURL(const char *url, char *hostname, unsigned short *port);
	static unsigned short getResponseCode(char *line);
	static int getLine(Tcp c, uint8_t *buffer, size_t maxsize, int toRead);
	static int getLine(Tcp c, uint8_t *buffer, size_t maxsize);
};

#endif
