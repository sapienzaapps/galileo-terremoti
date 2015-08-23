#include <vendor_specific.h>
#include "NetworkManager.h"
#include "../Log.h"
#ifdef __IS_GALILEO
#include "../Utils.h"
#endif

bool NetworkManager::connectionAvailable = false;
bool NetworkManager::connectionChecked = false;

bool NetworkManager::isConnectedToInternet() {
	return NetworkManager::isConnectedToInternet(false);
}

bool NetworkManager::isConnectedToInternet(bool force) {
	if (!NetworkManager::connectionChecked || force) {
		NetworkManager::connectionAvailable = false;

		int ping = system(CMD_PING);

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

void NetworkManager::init() {
	signal(SIGPIPE, SIG_IGN); // TODO: Remove? - caused not restarting sketch
#ifdef __IS_GALILEO
	Log::i("Fix Galileo bugs");
	// Workaround for Galileo (and other boards with Linux)
	system("/etc/init.d/networking restart");
	// Remove for production use
	//system("telnetd -l /bin/sh");
	delay(1000);
#endif
}
