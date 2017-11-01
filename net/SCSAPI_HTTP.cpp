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
	bool ret = error == HTTPError::HTTP_OK && resp->responseCode == 200;
	HTTPClient::freeHTTPResponse(resp);
	return ret;
}

bool SCSAPI_HTTP::alive() {
	bool ret = false;
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
		ret = true;
	} else {
		Log::e("Error connecting to HTTP server");
		cfg = "";
	}
	HTTPClient::freeHTTPResponse(resp);

	if (!cfg.empty()) {
		Config::parseServerConfig(cfg);
	}
	return ret;
}

bool SCSAPI_HTTP::terremoto(RECORD *db) {
	bool ret = false;
	std::map<std::string, std::string> postValues;
	postValues["tsstart"] = Utils::toString(db->ts);
	postValues["deviceid"] = Config::getMacAddress();
	HTTPResponse *resp = HTTPClient::httpRequest(HTTP_POST, HTTP_BASE + "terremoto.php", postValues);
	if (resp->error == HTTP_OK && resp->responseCode == 200 && resp->body != NULL) {
		//nextContact = atol((const char *) resp->body) * 1000UL;
		ret = true;
	} else {
		Log::e("Error connecting to HTTP server");
	}
	HTTPClient::freeHTTPResponse(resp);
	return ret;
}

void SCSAPI_HTTP::tick() {
}

bool SCSAPI_HTTP::requestTimeUpdate() {
	bool ret = false;
	HTTPResponse *resp = HTTPClient::httpRequest(HTTP_GET, HTTP_BASE + "ntp.php");
	if (resp->error == HTTP_OK && resp->responseCode == 200) {
		unsigned long lastNTPTime;
		std::string ntptime = std::string((char *) resp->body);
		lastNTPTime = strtoul(ntptime.c_str(), NULL, 10);
		Log::d("Time: %d", lastNTPTime);
		execSystemTimeUpdate(lastNTPTime);
		ret = true;
	} else {
		Log::e("Error connecting to HTTP server");
	}
	HTTPClient::freeHTTPResponse(resp);
	return ret;
}

bool SCSAPI_HTTP::ping() {
	bool ret = false;
	HTTPResponse *resp = HTTPClient::httpRequest(HTTP_GET, HTTP_BASE + "ntp.php");
	if (resp->error == HTTP_OK && resp->responseCode == 200) {
		ret = true;
	} else {
		Log::e("Error connecting to HTTP server");
	}
	HTTPClient::freeHTTPResponse(resp);
	return ret;
}

SCSAPI_HTTP::~SCSAPI_HTTP() = default;