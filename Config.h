#ifndef config_h
#define config_h

#include <sstream>
#include <vector>
#include <map>
#include "common.h"
#include "Seismometer.h"

/**
 * Configuration management class
 */
class Config {
public:
	/**
	 * Init config class
	 */
	static void init();

	/**
	 * Returns if MAC address is configured
	 * @param True if MAC is configured, false otherwise
	 */
	static bool hasMACAddress();

	/**
	 * Get MAC Address
	 * @return MAC Address
	 */
	static std::string getMacAddress();

	/**
	 * Get MAC Address as byte array
	 * @param mac Byte array where MAC is stored
	 */
	static void getMacAddressAsByte(byte mac[6]);

	/**
	 * Print config to log
	 */
	static void printConfig();

	/**
	 * Set MAC Address
	 * @param macAddress MAC Address to use
	 */
	static void setMacAddress(std::string macAddress);

	/**
	 * Returns the syslog server
	 * @return Syslog server
	 */
	static uint32_t getSyslogServer();

	/**
	 * Returns if there is a proxy server configured
	 */
	static bool hasProxyServer();

	/**
	 * Returns the proxy server address
	 */
	static std::string getProxyServer();

	/**
	 * Returns the proxy server port
	 */
	static uint16_t getProxyPort();

	/**
	 * Returns true if the proxy server needs auth
	 * Note: only Basic auth is supported
	 */
	static bool isProxyAuthenticated();

	/**
	 * Returns the proxy user name for auth
	 */
	static std::string getProxyUser();

	/**
	 * Returns the proxy user password for auth
	 */
	static std::string getProxyPass();

	/**
	 * Save config to file
	 */
	static void save();

	static bool parseServerConfig(std::string cfg);

private:
	static void loadDefault();
	static bool readConfigFile(const char *filepath);
	static std::map<std::string, std::string> &configSplit(const std::string &s, char delim, std::map<std::string, std::string> &elems);
	static std::map<std::string, std::string> configSplit(const std::string &s, char delim);
	static void file_put_contents(const char* path, std::string content);

	static std::string macAddress;
	static uint32_t syslogServer;
	static std::string proxyServer;
	static uint16_t proxyPort;
	static std::string proxyUser;
	static std::string proxyPass;
};

#endif
