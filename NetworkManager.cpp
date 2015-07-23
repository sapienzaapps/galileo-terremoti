
#include "NetworkManager.h"
#include "Log.h"

bool NetworkManager::isDhcpClient = true;
uint8_t *NetworkManager::mac = NULL;
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
	if (!NetworkManager::networkSetup) {
		return false;
	}
	if (!NetworkManager::connectionChecked || force) {
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

void NetworkManager::setupStatic(uint8_t *mac, IPAddress staticAddress, IPAddress subnetMask, IPAddress gateway,
								 IPAddress dnsHost) {
	Log::d("Static configuration: %i.%i.%i.%i/%i.%i.%i.%i gw %i.%i.%i.%i dns %i.%i.%i.%i",
		   staticAddress[0], staticAddress[1], staticAddress[2], staticAddress[3],
		   subnetMask[0], subnetMask[1], subnetMask[2], subnetMask[3],
		   gateway[0], gateway[1], gateway[2], gateway[3],
		   dnsHost[0], dnsHost[1], dnsHost[2], dnsHost[3]
	);

	if (NetworkManager::mac != NULL) {
		free(NetworkManager::mac);
		NetworkManager::mac = NULL;
	}
	NetworkManager::mac = (uint8_t *) malloc(6);
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
	init();
}

void NetworkManager::forceRestart() {
	bool connected;
	int retry = 1;
	do {
		Log::i("Network reset #%i", retry);
		NetworkManager::restart();
		delay(1000);
		connected = NetworkManager::isConnectedToInternet(true);
		if (retry > 5) {
			system("reboot");
			for (; ;);
		} else if (!connected) {
			retry++;
			sleep(2000);
		}
	} while (!connected);
}

void NetworkManager::init() {
#ifdef __IS_GALILEO
	Log::i("Fix Galileo bugs");
	// Fixing Arduino Galileo bug
	signal(SIGPIPE, SIG_IGN); // TODO: Remove? - caused not restarting sketch
	// Workaround for Galileo (and other boards with Linux)
	system("/etc/init.d/networking restart");
	// Remove for production use
	//system("telnetd -l /bin/sh");
	delay(1000);
#endif
	if (NetworkManager::isDhcpClient) {
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
