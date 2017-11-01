//
// Created by enrico on 11/1/17.
//

#include "SCSAPI_HTTP.h"
#include "HTTPClient.h"
#include "../Utils.h"
#include "../Log.h"
#include "../generic.h"

SCSAPI_HTTP::SCSAPI_HTTP() = default;

bool SCSAPI_HTTP::init() {
	HTTPResponse *resp = HTTPClient::httpRequest(HTTP_GET, HTTP_BASE);
	HTTPError error = resp->error;
	HTTPClient::freeHTTPResponse(resp);
	return error == HTTPError::HTTP_OK;
}

void SCSAPI_HTTP::alive() {
	std::string cfg;
	std::map<std::string, std::string> postValues;
	postValues["deviceid"] = Config::getMacAddress();
	postValues["version"] = SOFTWARE_VERSION;
	postValues["memfree"] = Utils::toString(Utils::getFreeRam());
	postValues["uptime"] = Utils::toString(Utils::uptime());
	postValues["model"] = PLATFORM_TAG;
	postValues["sensor"] = Seismometer::getInstance()->getAccelerometerName();

	HTTPResponse *resp = HTTPClient::httpRequest(HTTP_POST, HTTP_BASE + "alive.php", postValues);
	Log::d("Response received, code: %i", resp->responseCode);
	if (resp->error == HTTP_OK && resp->responseCode == 200 && resp->body != NULL) {
		cfg = std::string((char *) resp->body);
	} else {
		Log::e("Error connecting to HTTP server");
		cfg = "";
	}
	HTTPClient::freeHTTPResponse(resp);

	if (!cfg.empty()) {
		// TODO: Config
	}
}

void SCSAPI_HTTP::terremoto(RECORD *db) {
	std::map<std::string, std::string> postValues;
	postValues["tsstart"] = Utils::toString(db->ts);
	postValues["deviceid"] = Config::getMacAddress();
	HTTPResponse *resp = HTTPClient::httpRequest(HTTP_POST, HTTP_BASE + "terremoto.php", postValues);
	if (resp->error == HTTP_OK && resp->body != NULL) {
		//nextContact = atol((const char *) resp->body) * 1000UL;
	} else {
		Log::e("Error connecting to HTTP server");
		Utils::delay(5 * 1000);
		platformReboot();
	}
	HTTPClient::freeHTTPResponse(resp);
}

void SCSAPI_HTTP::tick() {
}

unsigned long SCSAPI_HTTP::getUNIXTime() {
	unsigned long diff = Utils::millis() - SCSAPI_HTTP::lastNTPMillis;
	return (SCSAPI_HTTP::lastNTPTime + (diff / 1000));
}

void SCSAPI_HTTP::requestTimeUpdate() {
	HTTPResponse *resp = HTTPClient::httpRequest(HTTP_GET, HTTP_BASE + "ntp.php");
	if (resp->error == HTTP_OK) {
		std::string ntptime = std::string((char *) resp->body);
		lastNTPTime = strtoul(ntptime.c_str(), NULL, 10);
		Log::d("Time: %d", lastNTPTime);
		lastNTPMillis = Utils::millis();
	} else {
		Log::e("Error connecting to HTTP server");
		Utils::delay(5 * 1000);
		platformReboot();
	}
	HTTPClient::freeHTTPResponse(resp);
}

bool SCSAPI_HTTP::ping() {
	// TODO
	return true;
}

SCSAPI_HTTP::~SCSAPI_HTTP() = default;