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
	 * Send crash reports to server
	 */
	static void sendCrashReports();

	/**
	 * Crash report thread function
	 * @param mem
	 * @return
	 */
	static void *sendCrashReportDoWork(void *mem);

	/**
	 * Send a file to server
	 * @param URL
	 * @param file
	 * @return
	 */
	static HTTPResponse *httpPostFile(std::string URL, std::string file);

	/**
	 * Make an HTTP Request
	 * @param method
	 * @param URL
	 * @param postValues
	 * @return
	 */
	static HTTPResponse *httpRequest(HTTPMethod method, std::string URL,
									 std::map<std::string, std::string> postValues);

	static HTTPResponse *httpRequest(HTTPMethod method, std::string URL);

	/**
	 * Free HTTPResponse
	 * @param resp
	 */
	static void freeHTTPResponse(HTTPResponse *resp);

	static int getLine(Tcp c, uint8_t *buffer, size_t maxsize, int toRead);

	static int getLine(Tcp c, uint8_t *buffer, size_t maxsize);

private:
	static pthread_t sendCrashReportThread;

	static size_t hostFromURL(const char *url, char *hostname, unsigned short *port);

	static unsigned short getResponseCode(char *line);
};

#endif
