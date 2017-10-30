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
	 * Get configured latitude
	 * @return Configured latitude
	 */
	static double getLatitude();

	/**
	 * Get configured longitude
	 * @return Configured longitude
	 */
	static double getLongitude();

	/**
	 * Device is configured with a GPS position?
	 * @return True if position is set, false otherwise
	 */
	static bool hasPosition();

	/**
	 * Check server config, then refresh locally
	 * @return True if config is OK, false if an error occurred
	 */
	static bool checkServerConfig();

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
	 * Set latitude
	 * @return Latitude to set
	 */
	static void setLatitude(double lat);

	/**
	 * Set longitude
	 * @return Longitude to set
	 */
	static void setLongitude(double lon);

	/**
	 * Returns the syslog server
	 * @return Syslog server
	 */
	static uint32_t getSyslogServer();

	/**
	 * Save config to file
	 */
	static void save();

private:
    /**
     * Load default values for config
     */
	static void loadDefault();

    /**
     * Read the config file
     */
	static bool readConfigFile(const char *filepath);

    /**
     * Functions used to parse the config string from server (deprecated)
     */
	static std::map<std::string, std::string> &configSplit(const std::string &s, char delim, std::map<std::string, std::string> &elems);
	static std::map<std::string, std::string> configSplit(const std::string &s, char delim);

    /**
     * Function used to save file
     */
    static void file_put_contents(const char* path, std::string content);

	static double lat;
	static double lon;
	static std::string macAddress;
	static uint32_t syslogServer;
};

#endif
