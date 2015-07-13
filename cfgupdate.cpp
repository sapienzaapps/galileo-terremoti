#include <sstream>
#include <vector>

#include "cfgupdate.h"
#include "commons.h"
#include "HTTPClient.h"
#include "NetworkManager.h"
#include "NTP.h"

unsigned long lastCfgUpdate = 0;
unsigned long cfgUpdateInterval = 60;

long previousMillisConfig = 0;        // will store last time LED was updated
long intervalConfig = 1 * 60 * 1000;// 3 minuti

IPAddress getFromString(char *ipAddr) {
	char *p1 = strchr(ipAddr, '.');
	char *p2 = strchr(p1 + 1, '.');
	char *p3 = strchr(p2 + 1, '.');

	*p1 = 0;
	*p2 = 0;
	*p3 = 0;

	uint32_t d1 = atoi(ipAddr);
	uint32_t d2 = atoi(p1 + 1);
	uint32_t d3 = atoi(p2 + 1);
	uint32_t d4 = atoi(p3 + 1);

	return (uint32_t) (d1 * 16777216 + d2 * 65536 + d3 * 256 + d4);
}

// ask config to server - New Da finire
boolean getConfigNew() {
	Log::i("getConfigNew()------------------ START ----------------------");
	boolean ret = false;

	std::string cfg = HTTPClient::getConfig();
	if (cfg != "") {
		ret = true;
		std::map<std::string, std::string> params = configSplit(cfg, '|');

		HTTPClient::setBaseURL(params["server"]);

		// Only IP Address!!
		IPAddress ntpserver((const uint8_t *)params["ntpserver"].c_str());
		NTP::setNTPServer(ntpserver);

		bool exec_status = false;
		std::string script = params["script"];
		if (!script.empty()) {
			char scriptTest[1024];
			strncpy(scriptTest, script.c_str(), (min(script.size(), 1024)));
			createScript("/media/realroot/script.sh", scriptTest);
			exec_status = true;
			Log::d("Script Creation...");
			Log::d("Script length: %i", script.size());
			Log::d("Script: %s", script.c_str());
		}

		std::string path = params["path"];
		if (!path.empty()) {
			char pathTest[1024];
			char pathScriptDownload[1024];
			strncpy(pathTest, path.c_str(), path.size()); // remote peth for file downloading
			snprintf(pathScriptDownload, 1024, download_scriptText, pathTest);

			Log::i("pathTest: %s", pathTest);
			Log::i("pathScriptDownload: %s", pathScriptDownload);

			createScript(NULL,
						 pathScriptDownload); // creation script for download a file from the path(internet resource)
			Log::d("execScript for Download....");
			execScript(script_path); // executing download of the file
			delay(1000);
			Log::d("Path length: %i", path.size());
			Log::d("path: %s", path.c_str());
		}

		if (exec_status) { // check for executing script command
			execScript("/media/realroot/script.sh");
			Log::d("execScript.... /media/realroot/script.sh");
			for (int x = 0; x < 3; x++) {
				resetBlink(0);
			}
		}
	}

	Log::d("Still running Config Update");
	if (NetworkManager::isConnectedToInternet()) {
		Log::d("isConnectedToInternet");
	}
	Log::d("lastCfgUpdate: %ld", lastCfgUpdate);
	Log::d("cfgUpdateInterval: %ld", cfgUpdateInterval);
	Log::d("getUNIXTime(): ", NTP::getUNIXTime());
	if (!ret) {
		Log::e("getConfigNew() Update ERROR!!!");
	}
	Log::i("getConfigNew()------------------ EXIT ----------------------");
	return ret;
}

// get the HTTP Server(default if not) and NTP Server
void initConfigUpdates() {
	if (NetworkManager::isConnectedToInternet() &&
		start) { // get config onli if Galileo is connected and lat/lon are setted
		boolean ret = getConfigNew();
		int nTimes = 0;
		while (!ret && (nTimes < 5)) {
			nTimes++;
			Log::d("Configuration update failed, retrying in 3 seconds...");
			delay(3000);
			ret = getConfigNew();
		}
		if (nTimes >= 5)
			Log::e("getConfigNew()  -  failed!!!!!");
	}
}
